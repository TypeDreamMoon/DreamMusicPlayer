#include "Classes/DreamMusicPlayerLyricTools.h"

#include "DreamMusicPlayerSettings.h"
#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicPlayerLyricFileParser.h"

TArray<FDreamMusicLyric> FDreamMusicPlayerLyricTools::LoadLyricFromFile(FString FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		DMP_LOG(Warning, TEXT("Lyric Load Failed. Lyric Path Is Not Valid!"))
		return {};
	}
	TArray<FDreamMusicLyric> Buffer;
	TArray<FString> Lines;
	FFileHelper::LoadFileToStringArray(Lines, *FilePath);
	for (auto Element : Lines)
	{
		Buffer.Add(FDreamMusicLyric(Element));
		DMP_LOG(Log, TEXT("Lyric : Time : %02d:%02d.%02d : Content : %s"), Buffer.Last().Timestamp.Minute,
		        Buffer.Last().Timestamp.Seconds, Buffer.Last().Timestamp.Millisecond, *Buffer.Last().Content);
	}

	return Buffer;
}

FDreamMusicLyric FDreamMusicPlayerLyricTools::GetCurrentLyric(FDreamMusicLyricTimestamp CurrentTime,
                                                              const TArray<FDreamMusicLyric>& Lyrics)
{
	// TODO : 这里的开销可能有些大 需要优化
	for (auto Element : Lyrics)
	{
		if (Element.Timestamp == CurrentTime)
			return Element;
	}
	return FDreamMusicLyric();
}

FDreamMusicLyricTimestamp FDreamMusicPlayerLyricTools::Conv_TimestampFromFloat(float Time)
{
	int M, S, MS;
	Time = FMath::Abs(Time);
	M = FMath::FloorToInt(Time / 60.f);
	S = FMath::FloorToInt(Time - (M * 60.f));
	MS = FMath::FloorToInt((Time - FMath::FloorToFloat(Time)) * 100.f);
	FDreamMusicLyricTimestamp CurrentTimeStamp(M, S, MS);
	return CurrentTimeStamp;
}

float FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(FDreamMusicLyricTimestamp Timestamp)
{
	return Timestamp.Minute * 60.f + Timestamp.Seconds + Timestamp.Millisecond / 100.f;
}

FString FDreamMusicPlayerLyricTools::GetLyricFilePath(FString FileName)
{
	FString LocalPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LocalPath);
	LocalPath /= FileName;
	return LocalPath;
}

TArray<FString> FDreamMusicPlayerLyricTools::GetLyricFileNames()
{
	TArray<FString> Names;
	TArray<FString> LrcFiles, AssFiles, SrtFiles;

	FString LongPath;
	FPackageName::TryConvertGameRelativePackagePathToLocalPath(GetDefault<UDreamMusicPlayerSettings>()->LyricContentPath.Path, LongPath);

	IFileManager::Get().FindFiles(LrcFiles, *LongPath, TEXT("*.lrc"));
	IFileManager::Get().FindFiles(AssFiles, *LongPath, TEXT("*.ass"));
	IFileManager::Get().FindFiles(SrtFiles, *LongPath, TEXT("*.srt"));

	Names.Append(LrcFiles);
	Names.Append(AssFiles);
	Names.Append(SrtFiles);

	return Names;
}

#define IS_DEBUG_PARSER true
#define DMP_LOG_DEBUG(V, F, ...) if (IS_DEBUG_PARSER) DMP_LOG(V, TEXT("[Parser Debug]:" F), __VA_ARGS__)

FDreamMusicPlayerLyricParser::FDreamMusicPlayerLyricParser(FString InFilePath, EDreamMusicPlayerLyricParseFileType InFileType, EDreamMusicPlayerLyricParseLineType InLineType, EDreamMusicPlayerLrcLyricType InLrcParseMethod)
{
	FilePath = InFilePath;
	FileType = InFileType;
	LineType = InLineType;
	LrcParseMethod = InLrcParseMethod;

	DMP_LOG_DEBUG(Log, TEXT("Initialize: FIlePath: %s FileType: %s LineType: %s LRC Parse Method: %s"),
		*FilePath,
		*UEnum::GetValueAsString(FileType),
		*UEnum::GetValueAsString(LineType),
		*UEnum::GetValueAsString(LrcParseMethod))
	
	BeginDecodeFile();
}

void FDreamMusicPlayerLyricParser::BeginDecodeFile()
{
	// Clear previous data
	ClearCachedLines();
	ClearLyrics();
	MetaData.Empty();

	// Check if file exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Lyric file not found: %s"), *FilePath);
		return;
	}

	// Load file content
	if (!FFileHelper::LoadFileToStringArray(CachedFileLines, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load lyric file: %s"), *FilePath);
		return;
	}

	if (!FFileHelper::LoadFileToString(CachedFileContent, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load lyric file: %s"), *FilePath);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Loaded lyric file with %d lines: %s"), CachedFileLines.Num(), *FilePath);

	// Re-initialize parser with new lines
	InitializeParser();

	// Parse the file
	if (Parser.IsValid())
	{
		Parser->Parse();
		Lyrics = Parser->GetParsedLyrics();

		for (const FDreamMusicLyric& Lyric : Lyrics)
		{
			DMP_LOG_DEBUG(Log, TEXT("Lyric : %s"), *Lyric.ToString())
		}

		// Sort lyrics by timestamp
		SortLyricsByTimestamp();

		UE_LOG(LogTemp, Log, TEXT("Successfully parsed %d lyrics from file"), Lyrics.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize parser for file type"));
	}
}

void FDreamMusicPlayerLyricParser::InitializeParser()
{
	if (CachedFileLines.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No cached lines available for parser initialization"));
		return;
	}

	// Create appropriate parser based on file type
	switch (FileType)
	{
	case EDreamMusicPlayerLyricParseFileType::SRT:
		Parser = MakeShared<FDreamMusicPlayerLyricFileParser_SRT>(CachedFileContent, CachedFileLines, LineType);
		break;

	case EDreamMusicPlayerLyricParseFileType::LRC:
		Parser = MakeShared<FDreamMusicPlayerLyricFileParser_LRC>(CachedFileContent, CachedFileLines, LrcParseMethod, LineType);
		break;

	case EDreamMusicPlayerLyricParseFileType::ASS:
		Parser = MakeShared<FDreamMusicPlayerLyricFileParser_ASS>(CachedFileContent, CachedFileLines, LineType);
		break;

	default:
		UE_LOG(LogTemp, Error, TEXT("Unsupported lyric file type"));
		break;
	}
}

void FDreamMusicPlayerLyricParser::ClearCachedLines()
{
	CachedFileContent.Empty();
	CachedFileLines.Empty();
}

void FDreamMusicPlayerLyricParser::ClearLyrics()
{
	Lyrics.Empty();
}

TArray<FDreamMusicLyric> FDreamMusicPlayerLyricParser::GetLyrics()
{
	return Lyrics;
}

void FDreamMusicPlayerLyricParser::SortLyricsByTimestamp()
{
	Lyrics.Sort([](const FDreamMusicLyric& A, const FDreamMusicLyric& B)
	{
		return A.Timestamp.TotalMilliseconds() < B.Timestamp.TotalMilliseconds();
	});
}

bool FDreamMusicPlayerLyricParser::IsValidLyricFile() const
{
	return !CachedFileLines.IsEmpty() && FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath);
}

FString FDreamMusicPlayerLyricParser::GetFileExtension() const
{
	FString Extension;
	FString FileName;
	FString BaseName;

	FPaths::Split(FilePath, BaseName, FileName, Extension);
	return Extension.ToLower();
}

EDreamMusicPlayerLyricParseFileType FDreamMusicPlayerLyricParser::DetectFileType() const
{
	FString Extension = GetFileExtension();

	if (Extension == TEXT("srt"))
	{
		return EDreamMusicPlayerLyricParseFileType::SRT;
	}
	else if (Extension == TEXT("ass") || Extension == TEXT("ssa"))
	{
		return EDreamMusicPlayerLyricParseFileType::ASS;
	}
	else if (Extension == TEXT("lrc"))
	{
		// Analyze content to determine LRC subtype
		return EDreamMusicPlayerLyricParseFileType::LRC;
	}

	// Default fallback
	return EDreamMusicPlayerLyricParseFileType::LRC;
}

EDreamMusicPlayerLrcLyricType FDreamMusicPlayerLyricParser::DetectLRCSubtype() const
{
	for (const FString& Line : CachedFileLines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		// Skip metadata lines by checking if line contains actual timestamp and content
		if (TrimmedLine.StartsWith(TEXT("[")))
		{
			// Simple detection: check for ESLyric format (contains <timestamp> tags within content)
			if (TrimmedLine.Contains(TEXT("<")) && TrimmedLine.Contains(TEXT(">")))
			{
				// Look for pattern like [mm:ss.mmm]content<mm:ss.mmm>
				int32 FirstBracket = TrimmedLine.Find(TEXT("]"));
				if (FirstBracket != INDEX_NONE)
				{
					FString AfterFirstTimestamp = TrimmedLine.Mid(FirstBracket + 1);
					if (AfterFirstTimestamp.Contains(TEXT("<")) && AfterFirstTimestamp.Contains(TEXT(">")))
					{
						return EDreamMusicPlayerLrcLyricType::ESLyric;
					}
				}
			}

			// Check for word-by-word format (multiple [timestamp] tags per line)
			int32 FirstTimestamp = TrimmedLine.Find(TEXT("]"));
			if (FirstTimestamp != INDEX_NONE)
			{
				FString AfterFirstTimestamp = TrimmedLine.Mid(FirstTimestamp + 1);
				if (AfterFirstTimestamp.Contains(TEXT("[")))
				{
					return EDreamMusicPlayerLrcLyricType::WordByWord;
				}
			}
		}
	}

	// Default to line-by-line
	return EDreamMusicPlayerLrcLyricType::LineByLine;
}

void FDreamMusicPlayerLyricParser::ExtractMetadata()
{
	MetaData.Empty();

	for (const FString& Line : CachedFileLines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		// LRC metadata format: [tag:value]
		if (TrimmedLine.StartsWith(TEXT("[")) && TrimmedLine.EndsWith(TEXT("]")))
		{
			FString Content = TrimmedLine.Mid(1, TrimmedLine.Len() - 2);
			FString Tag, Value;

			if (Content.Split(TEXT(":"), &Tag, &Value))
			{
				// Check if it's not a timestamp (metadata vs lyric line)
				if (!Tag.IsNumeric() && !Tag.Contains(TEXT(".")))
				{
					MetaData.Add(Tag.ToLower(), Value);
				}
			}
		}
	}
}

FString FDreamMusicPlayerLyricParser::GetMetadata(const FString& Key) const
{
	const FString* FoundValue = MetaData.Find(Key.ToLower());
	return FoundValue ? *FoundValue : FString();
}

float FDreamMusicPlayerLyricParser::GetTotalDuration() const
{
	if (Lyrics.IsEmpty())
		return 0.0f;

	// Find the latest end timestamp
	float MaxDuration = 0.0f;
	for (const FDreamMusicLyric& Lyric : Lyrics)
	{
		float EndTime = FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(Lyric.EndTimestamp);
		if (EndTime > MaxDuration)
		{
			MaxDuration = EndTime;
		}

		// Fallback to start timestamp if end timestamp is not set
		if (Lyric.EndTimestamp.TotalMilliseconds() == 0)
		{
			float StartTime = FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(Lyric.Timestamp);
			if (StartTime > MaxDuration)
			{
				MaxDuration = StartTime;
			}
		}
	}

	return MaxDuration;
}

bool FDreamMusicPlayerLyricParser::ValidateTimestamps() const
{
	if (Lyrics.IsEmpty())
		return true;

	for (int32 i = 0; i < Lyrics.Num() - 1; i++)
	{
		// Check if timestamps are in ascending order
		if (Lyrics[i].Timestamp.TotalMilliseconds() > Lyrics[i + 1].Timestamp.TotalMilliseconds())
		{
			return false;
		}

		// Check if end timestamp is after start timestamp
		if (Lyrics[i].EndTimestamp.TotalMilliseconds() > 0 &&
			Lyrics[i].Timestamp.TotalMilliseconds() >= Lyrics[i].EndTimestamp.TotalMilliseconds())
		{
			return false;
		}
	}

	return true;
}

TArray<FString> FDreamMusicPlayerLyricParser::GetValidationErrors() const
{
	TArray<FString> Errors;

	if (!IsValidLyricFile())
	{
		Errors.Add(TEXT("File does not exist or is not accessible"));
	}

	if (CachedFileLines.IsEmpty())
	{
		Errors.Add(TEXT("No content loaded from file"));
	}

	if (Lyrics.IsEmpty())
	{
		Errors.Add(TEXT("No lyrics parsed from file"));
	}

	if (!ValidateTimestamps())
	{
		Errors.Add(TEXT("Timestamps are not in correct order or have invalid ranges"));
	}

	// Check for missing content
	int32 EmptyLyrics = 0;
	for (const FDreamMusicLyric& Lyric : Lyrics)
	{
		if (Lyric.Content.IsEmpty() && !Lyric.bIsEmptyLine)
		{
			EmptyLyrics++;
		}
	}

	if (EmptyLyrics > 0)
	{
		Errors.Add(FString::Printf(TEXT("Found %d lyrics with empty content"), EmptyLyrics));
	}

	return Errors;
}
