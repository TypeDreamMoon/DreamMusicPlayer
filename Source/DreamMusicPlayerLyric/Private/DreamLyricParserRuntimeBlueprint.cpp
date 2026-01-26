#include "DreamLyricParserRuntimeBlueprint.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include <string>

#include "DreamLyricAsset.h"

#include "DreamLyricUtils.h"
#include "DreamLyricTypes.h"
#include "dlp/Process.hpp"

dlp::File::FLyricGroupRoleOption FDreamLyricParserOptions::ToLibraryType() const
{
	std::vector<dlp::ELyricContentRole> Roles;
	dlp::File::FLyricGroupRoleOption Result;

	for (auto elem : GroupingSequence)
	{
		std::vector<dlp::ELyricContentRole> cache;
		for (EDreamMusicLyricTextRole Role : elem.Roles)
		{
			cache.push_back(FDreamLyricUtils::ConvertRole(Role));
		}
		Result.AddRoleGroup(cache);
	}

	Result.SetFallbackRole(FDreamLyricUtils::ConvertRole(FallbackRole));

	return Result;
}

FDreamLyricImportResult UDreamLyricParserRuntimeBlueprint::ImportLyricFileFromPath(
	const FString& FilePath,
	EDreamMusicPlayerLyricType Format,
	const FDreamLyricParserOptions& ParserOptions)
{
	FDreamLyricImportResult Result;

	// 检查文件是否存在
	FString AbsolutePath = FPaths::ConvertRelativePathToFull(FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*AbsolutePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("文件不存在: %s"), *AbsolutePath);
		return Result;
	}

	// 读取文件内容
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *AbsolutePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("无法读取文件: %s"), *AbsolutePath);
		return Result;
	}

	// 创建资产对象
	UDreamLyricAsset* Asset = NewObject<UDreamLyricAsset>();
	Asset->SourceFileName = FPaths::GetCleanFilename(AbsolutePath);

	// 解析文件内容
	FString ParseError;
	dlp::EFileFormat ParserFormat = FDreamLyricUtils::ConvertFormat(Format);
	if (!ParseLyricContent(FileContent, ParserFormat, Asset, ParseError, ParserOptions))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("解析失败: %s"), *ParseError);
		return Result;
	}

	Result.bSuccess = true;
	Result.Asset = Asset;
	Result.ErrorMessage = TEXT("Successful");
	return Result;
}

FDreamLyricImportResult UDreamLyricParserRuntimeBlueprint::ImportLyricFileFromString(
	const FString& FileContent,
	EDreamMusicPlayerLyricType Format,
	const FString& SourceFileName,
	const FDreamLyricParserOptions& ParserOptions)
{
	FDreamLyricImportResult Result;

	// 创建资产对象
	UDreamLyricAsset* Asset = NewObject<UDreamLyricAsset>();
	Asset->SourceFileName = SourceFileName.IsEmpty() ? TEXT("ImportedFromString") : SourceFileName;

	// 解析文件内容
	FString ParseError;
	dlp::EFileFormat ParserFormat = FDreamLyricUtils::ConvertFormat(Format);
	if (!ParseLyricContent(FileContent, ParserFormat, Asset, ParseError, ParserOptions))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("解析失败: %s"), *ParseError);
		return Result;
	}

	Result.bSuccess = true;
	Result.Asset = Asset;
	Result.ErrorMessage = TEXT("Successful");
	return Result;
}

EDreamMusicPlayerLyricType UDreamLyricParserRuntimeBlueprint::DetectFileFormat(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	return DetectFormatFromExtension(Extension);
}

bool UDreamLyricParserRuntimeBlueprint::CanImportFile(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	return Extension == TEXT("lrc") || Extension == TEXT("ass") || Extension == TEXT("srt");
}

EDreamMusicPlayerLyricType UDreamLyricParserRuntimeBlueprint::DetectFormatFromExtension(const FString& Extension)
{
	FString LowerExtension = Extension.ToLower();
	if (LowerExtension == TEXT("lrc"))
	{
		return EDreamMusicPlayerLyricType::LRC;
	}
	else if (LowerExtension == TEXT("ass"))
	{
		return EDreamMusicPlayerLyricType::ASS;
	}
	else if (LowerExtension == TEXT("srt"))
	{
		return EDreamMusicPlayerLyricType::SRT;
	}

	return EDreamMusicPlayerLyricType::LRC;
}

bool UDreamLyricParserRuntimeBlueprint::ParseLyricContent(
	const FString& FileContent,
	dlp::EFileFormat Format,
	UDreamLyricAsset* OutAsset,
	FString& OutErrorMessage,
	const FDreamLyricParserOptions& ParserOptions)
{
	if (!OutAsset)
	{
		OutErrorMessage = TEXT("输出资产对象为空");
		return false;
	}

	// 转换为 std::string
	std::string ContentStr = TCHAR_TO_UTF8(*FileContent);

	// 使用 try-catch 来捕获异常
	try
	{
		dlp::FProcess Process;
		dlp::File::FLyricFile LyricFile = Process.Process(Format, ContentStr, ParserOptions.ToLibraryType());

		// 转换为资产数据
		ConvertParsedLyric(LyricFile, OutAsset);

		return true;
	}
	catch (const std::exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("解析异常: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		OutErrorMessage = TEXT("发生未知异常");
		return false;
	}
}

void UDreamLyricParserRuntimeBlueprint::ConvertParsedLyric(dlp::File::FLyricFile ParsedFile, TArray<FDreamMusicLyricGroup>& OutGroups)
{
	// 转换组
	for (const auto& Group : ParsedFile.groups)
	{
		FDreamMusicLyricGroup LyricGroup;

		// 转换时间戳
		LyricGroup.StartTimestamp = Group.group_time_start;
		LyricGroup.EndTimestamp = Group.group_time_end;

		// 转换行
		for (const auto& Line : Group.parsed_lines)
		{
			FDreamMusicLyricLine LyricLine;

			// 转换角色
			LyricLine.Role = FDreamLyricUtils::ConvertRole(Line.role);

			LyricLine.Text = UTF8_TO_TCHAR(Line.lyric.c_str());

			// 转换词
			for (const auto& Word : Line.parsed_words)
			{
				FDreamMusicLyricWord LyricWord;
				LyricWord.Content = UTF8_TO_TCHAR(Word.word.c_str());
				LyricWord.StartTimestamp = Word.time_start;
				LyricWord.EndTimestamp = Word.time_end;

				LyricLine.Words.Add(LyricWord);
			}

			LyricGroup.Lines.Add(LyricLine);
		}

		OutGroups.Add(LyricGroup);
	}
}

void UDreamLyricParserRuntimeBlueprint::ConvertParsedLyric(
	const dlp::File::FLyricFile& ParsedFile,
	UDreamLyricAsset* Asset)
{
	if (!Asset)
	{
		return;
	}

	// 转换元数据
	for (const auto& Pair : ParsedFile.metadata.metadata)
	{
		Asset->Metadata.Items.Add(UTF8_TO_TCHAR(Pair.first.c_str()), UTF8_TO_TCHAR(Pair.second.c_str()));
	}

	// 转换组
	for (const auto& Group : ParsedFile.groups)
	{
		FDreamMusicLyricGroup LyricGroup;

		// 转换时间戳
		LyricGroup.StartTimestamp = Group.group_time_start;
		LyricGroup.EndTimestamp = Group.group_time_end;

		// 转换行
		for (const auto& Line : Group.parsed_lines)
		{
			FDreamMusicLyricLine LyricLine;

			// 转换角色
			LyricLine.Role = FDreamLyricUtils::ConvertRole(Line.role);

			LyricLine.Text = UTF8_TO_TCHAR(Line.lyric.c_str());

			// 转换词
			for (const auto& Word : Line.parsed_words)
			{
				FDreamMusicLyricWord LyricWord;
				LyricWord.Content = UTF8_TO_TCHAR(Word.word.c_str());
				LyricWord.StartTimestamp = Word.time_start;
				LyricWord.EndTimestamp = Word.time_end;

				LyricLine.Words.Add(LyricWord);
			}

			LyricGroup.Lines.Add(LyricLine);
		}

		Asset->Groups.Add(LyricGroup);
	}
}
