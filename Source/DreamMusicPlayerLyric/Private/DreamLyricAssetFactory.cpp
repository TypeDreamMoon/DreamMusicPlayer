#include "DreamLyricAssetFactory.h"
#include "DreamLyricAsset.h"
#include "DreamLyricImportDialog.h"
#include "DreamLyricParserRuntimeBlueprint.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"
#include <string>

#include "DreamLyricTypes.h"
#include "DreamLyricUtils.h"
#include "dlp/Parser.hpp"
#include "dlp/Process.hpp"
#include "dlp/Parser/Parser_ASS.hpp"

#define LOCTEXT_NAMESPACE "DreamLyricAssetFactory"

ULyricAssetFactory::ULyricAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("lrc;LRC Lyrics File"));
	Formats.Add(TEXT("ass;ASS Subtitle File"));
	Formats.Add(TEXT("srt;SRT Subtitle File"));

	bCreateNew = false;
	bEditAfterNew = true;
	bEditorImport = true;
	SupportedClass = UDreamLyricAsset::StaticClass();
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
	dlp::EFileFormat ParserFormat;
	FString WindowTitle;
	FString FileFormatName;

	// 根据文件扩展名设置格式和窗口标题
	if (Extension == TEXT("lrc"))
	{
		ParserFormat = dlp::EFileFormat::LRC; // 默认值，会在对话框中修改
		WindowTitle = TEXT("Import LRC File");
		FileFormatName = TEXT("lrc");
	}
	else if (Extension == TEXT("ass"))
	{
		ParserFormat = dlp::EFileFormat::ASS;
		WindowTitle = TEXT("Import ASS File");
		FileFormatName = TEXT("ass");
	}
	else if (Extension == TEXT("srt"))
	{
		ParserFormat = dlp::EFileFormat::SRT;
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
	ImportDialog->SetFilePath(Filename); // 设置文件路径以启用预览

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(WindowTitle))
		.ClientSize(FVector2D(1000, 600))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::UserSized)
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
			ParserFormat = dlp::EFileFormat::ESLyric;
			break;
		case ELrcImportMode::WordByWord:
			ParserFormat = dlp::EFileFormat::LRC;
			break;
		case ELrcImportMode::LineByLine:
		default:
			ParserFormat = dlp::EFileFormat::LRC;
			break;
		}
	}

	// 创建资产
	UDreamLyricAsset* Asset = NewObject<UDreamLyricAsset>(InParent, InClass, InName, Flags);
	Asset->SourceFileName = FPaths::GetCleanFilename(Filename);

	if (Extension != TEXT("ass"))
	{
		// 获取用户配置的解析选项
		FDreamLyricParserOptions ParserOptions = ImportDialog->GetParserOptions();
		UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Starting file import with custom parser options"));
		if (!ImportLyricFile(Filename, Asset, ParserFormat, ParserOptions))
		{
			UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to import lyric file: %s"), *Filename);
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import lyric file: %s"), *Filename);
			return nullptr;
		}
	}
	else
	{
		FString Key_Lyric, Key_Romanization, Key_Translation;
		ImportDialog->GetAssFileRoleKeys(Key_Lyric, Key_Translation, Key_Romanization);
		dlp::Parser::FParserOptions* ParserOptions = dlp::Parser::FParserOptions::NewOption<dlp::Parser::FParserOptions_ASS>(TCHAR_TO_UTF8(*Key_Lyric), TCHAR_TO_UTF8(*Key_Romanization), TCHAR_TO_UTF8(*Key_Translation));
		if (!ImportLyricFile(Filename, Asset, ParserFormat, ParserOptions))
		{
			UE_LOG(LogTemp, Error, TEXT("LyricAssetFactory: Failed to import lyric file: %s"), *Filename);
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to import lyric file: %s"), *Filename);
			return nullptr;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("LyricAssetFactory: Import successful, created asset with %d groups"), Asset->Groups.Num());
	return Asset;
}

bool ULyricAssetFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename).ToLower();
	return Extension == TEXT("lrc") || Extension == TEXT("ass") || Extension == TEXT("srt");
}

FText ULyricAssetFactory::GetDisplayName() const
{
	return LOCTEXT("FactoryDisplayName", "Dream Lyric Asset");
}

bool ULyricAssetFactory::ImportLyricFile(const FString& Filename,
                                         UDreamLyricAsset* Asset,
                                         dlp::EFileFormat Format,
                                         const FDreamLyricParserOptions& ParserOptions)
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

	// 使用 try-catch 来捕获异常
	try
	{
		// 创建解析器
		// auto Parser = dream_lyric_parser::parser::FParserFactory::CreateParser(Format);
		dlp::FProcess Process;
		dlp::File::FLyricFile ParsedLyricFile = Process.Process(Format, ContentStr, ParserOptions.ToLibraryType());

		// 转换为资产数据
		ConvertParsedLyricToAsset(ParsedLyricFile, Asset);

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

bool ULyricAssetFactory::ImportLyricFile(const FString& Filename, UDreamLyricAsset* Asset, dlp::EFileFormat Format, dlp::Parser::FParserOptions* ParserOptions)
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

	// 使用 try-catch 来捕获异常
	try
	{
		// 创建解析器
		// auto Parser = dream_lyric_parser::parser::FParserFactory::CreateParser(Format);
		dlp::FProcess Process;
		dlp::File::FLyricFile ParsedLyricFile = Process.Process(Format, ContentStr, ParserOptions);

		// 转换为资产数据
		ConvertParsedLyricToAsset(ParsedLyricFile, Asset);

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

void ULyricAssetFactory::ConvertParsedLyricToAsset(
	const dlp::File::FLyricFile& ParsedFile,
	UDreamLyricAsset* Asset)
{
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
		LyricGroup.StartTimestamp = FDreamMusicTimestamp(Group.group_time_start);
		LyricGroup.EndTimestamp = FDreamMusicTimestamp(Group.group_time_end);

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

#undef LOCTEXT_NAMESPACE
