#include "LyricAssetFactory.h"
#include "LyricAsset.h"
#include "LyricImportDialog.h"
#include "DreamLyricParserRuntime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include "DreamLyricParser/DreamLyricParser.hpp"
#include <string>

ULyricAssetFactory::ULyricAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("lrc;LRC Lyrics File"));
	Formats.Add(TEXT("ass;ASS Subtitle File"));
	Formats.Add(TEXT("srt;SRT Subtitle File"));
	
	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = ULyricAsset::StaticClass();
}

UObject* ULyricAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Starting import of file: %s"), *Filename);

	// 读取文件内容
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *Filename))
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to read file: %s"), *Filename);
		Warn->Logf(ELogVerbosity::Error, TEXT("Failed to read file: %s"), *Filename);
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: File read successfully, size: %d bytes"), FileContent.Len());

	// 确定文件格式
	FString Extension = FPaths::GetExtension(Filename).ToLower();
	dream_lyric_parser::FParserFormat ParserFormat;
	FString WindowTitle;
	FString FileFormatName;

	// 根据文件扩展名设置格式和窗口标题
	if (Extension == TEXT("lrc"))
	{
		ParserFormat = dream_lyric_parser::FParserFormat::LrcLineByLine; // 默认值，会在对话框中修改
		WindowTitle = TEXT("Import LRC File");
		FileFormatName = TEXT("lrc");
	}
	else if (Extension == TEXT("ass"))
	{
		ParserFormat = dream_lyric_parser::FParserFormat::Ass;
		WindowTitle = TEXT("Import ASS File");
		FileFormatName = TEXT("ass");
	}
	else if (Extension == TEXT("srt"))
	{
		ParserFormat = dream_lyric_parser::FParserFormat::Srt;
		WindowTitle = TEXT("Import SRT File");
		FileFormatName = TEXT("srt");
	}
	else
	{
		Warn->Logf(ELogVerbosity::Error, TEXT("Unsupported file format: %s"), *Extension);
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: %s file detected, showing import dialog"), *FileFormatName.ToUpper());
	
	// 显示对话框选择导入选项
	TSharedPtr<SLyricImportDialog> ImportDialog = SNew(SLyricImportDialog);
	ImportDialog->SetFileFormat(FileFormatName);
	
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(WindowTitle))
		.ClientSize(FVector2D(450, 400))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::Autosized)
		[
			ImportDialog.ToSharedRef()
		];

	// 设置父窗口引用，以便对话框可以关闭窗口
	ImportDialog->SetParentWindow(Window);

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Adding modal window"));
	FSlateApplication::Get().AddModalWindow(Window, nullptr);
	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Modal window closed, should import: %d"), ImportDialog->ShouldImport() ? 1 : 0);

	if (!ImportDialog->ShouldImport())
	{
		UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Import cancelled by user"));
		bOutOperationCanceled = true;
		return nullptr;
	}

	// 对于 LRC 文件，根据选择的模式设置解析格式
	if (Extension == TEXT("lrc"))
	{
		ELrcImportMode SelectedMode = ImportDialog->GetSelectedMode();
		switch (SelectedMode)
		{
		case ELrcImportMode::ESLyric:
			ParserFormat = dream_lyric_parser::FParserFormat::LrcEsLyric;
			break;
		case ELrcImportMode::WordByWord:
			ParserFormat = dream_lyric_parser::FParserFormat::LrcWordByWord;
			break;
		case ELrcImportMode::LineByLine:
		default:
			ParserFormat = dream_lyric_parser::FParserFormat::LrcLineByLine;
			break;
		}
	}
	
	// 获取用户配置的解析选项
	FDreamLyricParserOptions ParserOptions = ImportDialog->GetParserOptions();
	
	// 创建资产
	ULyricAsset* Asset = NewObject<ULyricAsset>(InParent, InClass, InName, Flags);
	Asset->SourceFileName = FPaths::GetCleanFilename(Filename);

	// 导入文件（使用用户配置的解析选项）
	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Starting file import with custom parser options"));
	if (!ImportLyricFile(Filename, Asset, ParserFormat, ParserOptions))
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to import lyric file: %s"), *Filename);
		Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import lyric file: %s"), *Filename);
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Import successful, created asset with %d groups"), Asset->Groups.Num());
	return Asset;
}

bool ULyricAssetFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename).ToLower();
	return Extension == TEXT("lrc") || Extension == TEXT("ass") || Extension == TEXT("srt");
}

bool ULyricAssetFactory::ImportLyricFile(const FString& Filename, ULyricAsset* Asset, dream_lyric_parser::FParserFormat Format, const FDreamLyricParserOptions& ParserOptions)
{
	if (!Asset)
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Asset is null"));
		return false;
	}

	// 读取文件内容
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *Filename))
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to read file: %s"), *Filename);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: File read successfully, size: %d bytes"), FileContent.Len());

	// 转换为std::string
	std::string ContentStr = TCHAR_TO_UTF8(*FileContent);

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Attempting to create parser for format: %d"), (int32)Format);

	// 验证第三方库（参考 DreamLyricParserRuntime 的实现）
	FString LibraryError;
	if (!UDreamLyricParserRuntime::ValidateThirdPartyLibrary(LibraryError))
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: %s"), *LibraryError);
		return false;
	}

	// 使用 try-catch 来捕获异常
	try
	{
		// 创建解析器
		auto Parser = dream_lyric_parser::CreateParser(Format);
		if (!Parser)
		{
			UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to create parser"));
			return false;
		}

		// 检查是否可以解析
		if (!Parser->CanParse(ContentStr))
		{
			UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Parser cannot parse this content"));
			return false;
		}

		// 解析文件 - 使用用户定义的解析选项
		dream_lyric_parser::FParserOptions Options = UDreamLyricParserRuntime::ConvertParserOptions(ParserOptions);
		
		// 执行解析
		dream_lyric_parser::FParsedLyric ParsedLyric = Parser->Parse(ContentStr, Options);

		// 转换为资产数据
		ConvertParsedLyricToAsset(ParsedLyric, Asset);

		UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Successfully parsed and converted lyric file with %d groups"), Asset->Groups.Num());
		return true;
	}
	catch (const std::exception& e)
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Exception occurred: %s"), UTF8_TO_TCHAR(e.what()));
		return false;
	}
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Unknown exception occurred while importing lyric file. This may be due to missing DreamLyricParser.dll or incompatible DLL version."));
		return false;
	}
}

void ULyricAssetFactory::ConvertParsedLyricToAsset(const dream_lyric_parser::FParsedLyric& ParsedLyric, ULyricAsset* Asset)
{
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

