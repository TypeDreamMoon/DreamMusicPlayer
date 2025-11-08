#include "DreamLyricParserRuntime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include <string>

FDreamLyricImportResult UDreamLyricParserRuntime::ImportLyricFileFromPath(
	const FString& FilePath,
	EDreamLyricParserFormat Format,
	const FDreamLyricParserOptions& ParserOptions)
{
	FDreamLyricImportResult Result;

	// 验证第三方库
	FString LibraryError;
	if (!ValidateThirdPartyLibrary(LibraryError))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("第三方库验证失败: %s"), *LibraryError);
		return Result;
	}

	// 检查文件是否存在
	FString AbsolutePath = FPaths::ConvertRelativePathToFull(FilePath);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*AbsolutePath))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("文件不存在: %s"), *AbsolutePath);
		return Result;
	}

	// 如果格式为默认值，尝试自动检测
	if (Format == EDreamLyricParserFormat::LrcLineByLine)
	{
		Format = DetectFileFormat(AbsolutePath);
		if (Format == EDreamLyricParserFormat::LrcLineByLine)
		{
			// 如果检测失败，尝试根据扩展名判断
			FString Extension = FPaths::GetExtension(AbsolutePath).ToLower();
			Format = DetectFormatFromExtension(Extension);
		}
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
	ULyricAsset* Asset = NewObject<ULyricAsset>();
	Asset->SourceFileName = FPaths::GetCleanFilename(AbsolutePath);

	// 解析文件内容
	FString ParseError;
	dream_lyric_parser::FParserFormat ParserFormat = ConvertFormat(Format);
	if (!ParseLyricContent(FileContent, ParserFormat, Asset, ParseError, ParserOptions))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("解析失败: %s"), *ParseError);
		return Result;
	}

	Result.bSuccess = true;
	Result.Asset = Asset;
	Result.ErrorMessage = TEXT("");
	return Result;
}

FDreamLyricImportResult UDreamLyricParserRuntime::ImportLyricFileFromString(
	const FString& FileContent,
	EDreamLyricParserFormat Format,
	const FString& SourceFileName,
	const FDreamLyricParserOptions& ParserOptions)
{
	FDreamLyricImportResult Result;

	// 验证第三方库
	FString LibraryError;
	if (!ValidateThirdPartyLibrary(LibraryError))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("第三方库验证失败: %s"), *LibraryError);
		return Result;
	}

	// 创建资产对象
	ULyricAsset* Asset = NewObject<ULyricAsset>();
	Asset->SourceFileName = SourceFileName.IsEmpty() ? TEXT("ImportedFromString") : SourceFileName;

	// 解析文件内容
	FString ParseError;
	dream_lyric_parser::FParserFormat ParserFormat = ConvertFormat(Format);
	if (!ParseLyricContent(FileContent, ParserFormat, Asset, ParseError, ParserOptions))
	{
		Result.bSuccess = false;
		Result.ErrorMessage = FString::Printf(TEXT("解析失败: %s"), *ParseError);
		return Result;
	}

	Result.bSuccess = true;
	Result.Asset = Asset;
	Result.ErrorMessage = TEXT("");
	return Result;
}

EDreamLyricParserFormat UDreamLyricParserRuntime::DetectFileFormat(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	return DetectFormatFromExtension(Extension);
}

bool UDreamLyricParserRuntime::CanImportFile(const FString& FilePath)
{
	FString Extension = FPaths::GetExtension(FilePath).ToLower();
	return Extension == TEXT("lrc") || Extension == TEXT("ass") || Extension == TEXT("srt");
}

dream_lyric_parser::FParserFormat UDreamLyricParserRuntime::ConvertFormat(EDreamLyricParserFormat Format)
{
	switch (Format)
	{
	case EDreamLyricParserFormat::LrcLineByLine:
		return dream_lyric_parser::FParserFormat::LrcLineByLine;
	case EDreamLyricParserFormat::LrcWordByWord:
		return dream_lyric_parser::FParserFormat::LrcWordByWord;
	case EDreamLyricParserFormat::LrcEsLyric:
		return dream_lyric_parser::FParserFormat::LrcEsLyric;
	case EDreamLyricParserFormat::Srt:
		return dream_lyric_parser::FParserFormat::Srt;
	case EDreamLyricParserFormat::Ass:
		return dream_lyric_parser::FParserFormat::Ass;
	default:
		return dream_lyric_parser::FParserFormat::LrcLineByLine;
	}
}

EDreamLyricParserFormat UDreamLyricParserRuntime::DetectFormatFromExtension(const FString& Extension)
{
	FString LowerExtension = Extension.ToLower();
	if (LowerExtension == TEXT("lrc"))
	{
		return EDreamLyricParserFormat::LrcLineByLine;
	}
	else if (LowerExtension == TEXT("ass") || LowerExtension == TEXT("ssa"))
	{
		return EDreamLyricParserFormat::Ass;
	}
	else if (LowerExtension == TEXT("srt"))
	{
		return EDreamLyricParserFormat::Srt;
	}
	
	return EDreamLyricParserFormat::LrcLineByLine;
}

bool UDreamLyricParserRuntime::ParseLyricContent(
	const FString& FileContent,
	dream_lyric_parser::FParserFormat Format,
	ULyricAsset* OutAsset,
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
		// 创建解析器
		auto Parser = dream_lyric_parser::CreateParser(Format);
		if (!Parser)
		{
			OutErrorMessage = TEXT("无法创建解析器");
			return false;
		}

		// 检查是否可以解析
		if (!Parser->CanParse(ContentStr))
		{
			OutErrorMessage = TEXT("解析器无法解析此内容");
			return false;
		}

		// 解析文件 - 使用用户定义的解析选项
		dream_lyric_parser::FParserOptions Options = ConvertParserOptions(ParserOptions);
		
		// 执行解析
		dream_lyric_parser::FParsedLyric ParsedLyric = Parser->Parse(ContentStr, Options);

		// 转换为资产数据
		ConvertParsedLyricToAsset(ParsedLyric, OutAsset);

		return true;
	}
	catch (const std::exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("解析异常: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		OutErrorMessage = TEXT("发生未知异常，可能是 DLL 版本不兼容或缺失");
		return false;
	}
}

void UDreamLyricParserRuntime::ConvertParsedLyricToAsset(
	const dream_lyric_parser::FParsedLyric& ParsedLyric,
	ULyricAsset* Asset)
{
	if (!Asset)
	{
		return;
	}

	// 转换元数据
	for (const auto& Pair : ParsedLyric.metadata.items)
	{
		Asset->Metadata.Items.Add(UTF8_TO_TCHAR(Pair.first.c_str()), UTF8_TO_TCHAR(Pair.second.c_str()));
	}

	// 转换组
	for (const auto& Group : ParsedLyric.groups)
	{
		FLyricGroup LyricGroup;
		
		// 转换时间戳
		LyricGroup.Timestamp = FLyricTimeSpan(
			Group.timestamp.hours,
			Group.timestamp.minutes,
			Group.timestamp.seconds,
			Group.timestamp.milliseconds
		);

		// 转换行
		for (const auto& Line : Group.lines)
		{
			FLyricLine LyricLine;
			
			// 转换角色
			switch (Line.role)
			{
			case dream_lyric_parser::FLyricTextRole::Lyric:
				LyricLine.Role = ELyricTextRole::Lyric;
				break;
			case dream_lyric_parser::FLyricTextRole::Romanization:
				LyricLine.Role = ELyricTextRole::Romanization;
				break;
			case dream_lyric_parser::FLyricTextRole::Translation:
				LyricLine.Role = ELyricTextRole::Translation;
				break;
			default:
				LyricLine.Role = ELyricTextRole::None;
				break;
			}

			LyricLine.Text = UTF8_TO_TCHAR(Line.text.c_str());

			// 转换词
			for (const auto& Word : Line.words)
			{
				FLyricWord LyricWord;
				LyricWord.Text = UTF8_TO_TCHAR(Word.text.c_str());
				LyricWord.StartTime = FLyricTimeSpan(
					Word.start_time.hours,
					Word.start_time.minutes,
					Word.start_time.seconds,
					Word.start_time.milliseconds
				);

				if (Word.end_time.has_value())
				{
					LyricWord.bHasEndTime = true;
					LyricWord.EndTime = FLyricTimeSpan(
						Word.end_time->hours,
						Word.end_time->minutes,
						Word.end_time->seconds,
						Word.end_time->milliseconds
					);
				}
				else
				{
					LyricWord.bHasEndTime = false;
				}

				// 转换角色
				switch (Word.role)
				{
				case dream_lyric_parser::FLyricTextRole::Lyric:
					LyricWord.Role = ELyricTextRole::Lyric;
					break;
				case dream_lyric_parser::FLyricTextRole::Romanization:
					LyricWord.Role = ELyricTextRole::Romanization;
					break;
				case dream_lyric_parser::FLyricTextRole::Translation:
					LyricWord.Role = ELyricTextRole::Translation;
					break;
				default:
					LyricWord.Role = ELyricTextRole::None;
					break;
				}

				LyricLine.Words.Add(LyricWord);
			}

			LyricGroup.Lines.Add(LyricLine);
		}

		Asset->Groups.Add(LyricGroup);
	}
}

bool UDreamLyricParserRuntime::ValidateThirdPartyLibrary(FString& OutErrorMessage)
{
	// 尝试创建解析器以验证 DLL 是否可用（这是最直接的方法）
	// 如果 DLL 已经通过 RuntimeDependencies 复制到输出目录，应该可以直接使用
	try
	{
		auto TestParser = dream_lyric_parser::CreateParser(dream_lyric_parser::FParserFormat::LrcLineByLine);
		if (!TestParser)
		{
			OutErrorMessage = TEXT("无法创建解析器，DLL 可能不兼容或未正确加载");
			return false;
		}
	}
	catch (const std::exception& e)
	{
		OutErrorMessage = FString::Printf(TEXT("DLL 加载失败: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		// 如果创建解析器失败，尝试查找 DLL 文件位置以提供更好的错误信息
		FString DllPath = FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries"), TEXT("Win64"), TEXT("DreamLyricParser.dll"));
		if (!FPaths::FileExists(DllPath))
		{
			// 尝试在项目目录中查找
			DllPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"), TEXT("DreamLyricParser.dll"));
			if (!FPaths::FileExists(DllPath))
			{
				// 尝试在插件目录中查找
				FString PluginDllPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("DreamMusicPlayer"), TEXT("Binaries"), TEXT("Win64"), TEXT("DreamLyricParser.dll"));
				if (!FPaths::FileExists(PluginDllPath))
				{
					OutErrorMessage = TEXT("DreamLyricParser.dll 未找到。请确保 DLL 位于 Engine/Binaries/Win64/、Project/Binaries/Win64/ 或插件 Binaries 目录中");
					return false;
				}
			}
		}
		
		OutErrorMessage = TEXT("DLL 加载失败或版本不兼容。请检查 DLL 是否正确编译并与当前引擎版本兼容");
		return false;
	}

	return true;
}

dream_lyric_parser::FParserOptions UDreamLyricParserRuntime::ConvertParserOptions(const FDreamLyricParserOptions& UEOptions)
{
	dream_lyric_parser::FParserOptions Options;
	
	// 如果分组序列为空，直接使用默认规则（参考 CLI 示例的 ConfigureGrouping 函数）
	if (UEOptions.GroupingSequence.Num() == 0)
	{
		Options.grouping = dream_lyric_parser::FGroupingRule::Default();
		return Options;
	}
	
	// 创建自定义分组规则
	dream_lyric_parser::FGroupingRule GroupingRule;
	
	// 转换分组序列（参考 CLI 示例的 ConfigureGrouping 函数）
	for (ELyricTextRole Role : UEOptions.GroupingSequence)
	{
		// 跳过 None 角色
		if (Role == ELyricTextRole::None)
		{
			continue;
		}
		
		dream_lyric_parser::FLyricTextRole ParserRole;
		switch (Role)
		{
		case ELyricTextRole::Lyric:
			ParserRole = dream_lyric_parser::FLyricTextRole::Lyric;
			break;
		case ELyricTextRole::Romanization:
			ParserRole = dream_lyric_parser::FLyricTextRole::Romanization;
			break;
		case ELyricTextRole::Translation:
			ParserRole = dream_lyric_parser::FLyricTextRole::Translation;
			break;
		default:
			// 跳过无效角色
			continue;
		}
		GroupingRule.sequence.push_back(ParserRole);
	}
	
	// 如果转换后序列为空，使用默认规则
	if (GroupingRule.sequence.empty())
	{
		Options.grouping = dream_lyric_parser::FGroupingRule::Default();
		return Options;
	}
	
	// 转换回退角色
	switch (UEOptions.FallbackRole)
	{
	case ELyricTextRole::Lyric:
		GroupingRule.fallback = dream_lyric_parser::FLyricTextRole::Lyric;
		break;
	case ELyricTextRole::Romanization:
		GroupingRule.fallback = dream_lyric_parser::FLyricTextRole::Romanization;
		break;
	case ELyricTextRole::Translation:
		GroupingRule.fallback = dream_lyric_parser::FLyricTextRole::Translation;
		break;
	default:
		// 如果回退角色无效，使用 Lyric 作为默认值
		GroupingRule.fallback = dream_lyric_parser::FLyricTextRole::Lyric;
		break;
	}
	
	// 使用配置的分组规则
	Options.grouping = GroupingRule;
	
	return Options;
}

