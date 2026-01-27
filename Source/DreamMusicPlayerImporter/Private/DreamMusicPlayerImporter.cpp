#include "DreamMusicPlayerImporter.h"

#include "DreamLyricAsset.h"
#include "Classes/DreamMusicDataAsset.h" // 引入 Music Asset
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "IContentBrowserSingleton.h"
#include "IDesktopPlatform.h"
#include "Lyric/DreamLyricAssetFactory.h"
#include "Music/DreamMusicAssetFactory.h"

static const FName ImportLyricFileMenuName = TEXT("ImportLyricFile");
static const FName ImportMusicFileMenuName = TEXT("ImportMusicFile"); // 新增

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerImporterModule"

void FDreamMusicPlayerImporterModule::StartupModule()
{
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerImporter module started"));

#if WITH_EDITOR
	RegisterMenus();
#endif
}

void FDreamMusicPlayerImporterModule::ShutdownModule()
{
#if WITH_EDITOR
	UnregisterMenus();
#endif
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerImporter module shutdown"));
}


void FDreamMusicPlayerImporterModule::RegisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	FToolMenuOwner Owner = FToolMenuOwner(this);
	UToolMenu* Menu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools");

	FToolMenuSection& Section = Menu->AddSection("DreamMusicPlayerImport", LOCTEXT("DreamMusicPlayerImportSection", "Dream Music Player Import"));

	// Lyric Import Entry
	Section.AddMenuEntry(
		ImportLyricFileMenuName,
		LOCTEXT("ImportLyricFile", "Import Lyric File"),
		LOCTEXT("ImportLyricFileTooltip", "Import a lyric file (LRC, ASS, or SRT) as a Lyric Asset"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
		FUIAction(FExecuteAction::CreateRaw(this, &FDreamMusicPlayerImporterModule::OnImportLyricFileClicked))
	);

	// Music Import Entry (新增)
	Section.AddMenuEntry(
		ImportMusicFileMenuName,
		LOCTEXT("ImportMusicFile", "Import Music Asset"),
		LOCTEXT("ImportMusicFileTooltip", "Import a music file (MP3, FLAC, WAV) creating a DataAsset, Audio and Cover Art"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "ContentBrowser.AssetActions.Import"),
		FUIAction(FExecuteAction::CreateRaw(this, &FDreamMusicPlayerImporterModule::OnImportMusicFileClicked))
	);
}

void FDreamMusicPlayerImporterModule::UnregisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		ToolMenus->RemoveSection("LevelEditor.MainMenu.Tools", "DreamMusicPlayerImport");
	}
}

// ... OnImportLyricFileClicked implementation remains same ...
void FDreamMusicPlayerImporterModule::OnImportLyricFileClicked()
{
	// (保持原有代码不变)
	// 打开文件选择对话框
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) return;

	const FString FileTypes = TEXT("Lyric Files (*.lrc;*.ass;*.srt)|*.lrc;*.ass;*.srt|All Files (*.*)|*.*");
	TArray<FString> OutFilenames;
	if (!DesktopPlatform->OpenFileDialog(nullptr, LOCTEXT("ImportTitle", "Import Lyric").ToString(), FPaths::ProjectContentDir(), TEXT(""), FileTypes, 0, OutFilenames)) return;
	if (OutFilenames.Num() == 0) return;

	FString TargetPath = TEXT("/Game");
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedPaths;
	ContentBrowser.GetSelectedPathViewFolders(SelectedPaths);
	if (SelectedPaths.Num() > 0) TargetPath = SelectedPaths[0];

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	ULyricAssetFactory* Factory = NewObject<ULyricAssetFactory>();

	AssetToolsModule.Get().ImportAssets(OutFilenames, TargetPath, Factory, true);
}

// New Function
void FDreamMusicPlayerImporterModule::OnImportMusicFileClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get DesktopPlatform"));
		return;
	}

	// 音乐文件过滤器
	// TEXT("Music Files (*.mp3;*.flac;*.wav;*.ogg)|*.mp3;*.flac;*.wav;*.ogg;|All Files (*.*)|*.*")
	const TSet<FString> SupportFiles = {
		TEXT("mp3"),
		TEXT("flac"),
		TEXT("wav"),
		TEXT("ogg"),

		TEXT("ncm"), // 网易云
		TEXT("qmcflac"), // QQ音乐
		TEXT("mflac"), // QQ音乐
		TEXT("qmc"), // QQ音乐
		TEXT("mgg"), // QQ音乐
		TEXT("kgm"), // 酷狗
		TEXT("kwm"), // 酷我
		TEXT("xm"), // 虾米
	};
	FString FileTypes = TEXT("Music Files (");
	for (const FString& FileType : SupportFiles)
	{
		FileTypes += TEXT("*.");
		FileTypes += FileType;
		FileTypes += TEXT(";");
	}
	FileTypes += TEXT(")|");
	for (const FString& FileType : SupportFiles)
	{
		FileTypes += TEXT("*.");
		FileTypes += FileType;
		FileTypes += TEXT(";");
	}

	TArray<FString> OutFilenames;
	bool bFileSelected = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		LOCTEXT("ImportMusicFileDialogTitle", "Import Music File").ToString(),
		FPaths::ProjectContentDir(),
		TEXT(""),
		FileTypes,
		0,
		OutFilenames
	);

	if (!bFileSelected || OutFilenames.Num() == 0) return;

	// 确定路径
	FString TargetPath = TEXT("/Game");
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedPaths;
	ContentBrowser.GetSelectedPathViewFolders(SelectedPaths);
	if (SelectedPaths.Num() > 0 && SelectedPaths[0].StartsWith(TEXT("/Game")))
	{
		TargetPath = SelectedPaths[0];
	}

	// 执行导入
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// 使用新的 Music Factory
	UDreamMusicAssetFactory* Factory = NewObject<UDreamMusicAssetFactory>();

	AssetToolsModule.Get().ImportAssets(
		OutFilenames,
		TargetPath,
		Factory,
		true
	);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerImporterModule, DreamMusicPlayerImporter)
