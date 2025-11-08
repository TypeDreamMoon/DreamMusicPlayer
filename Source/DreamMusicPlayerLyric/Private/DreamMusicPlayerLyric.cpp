#include "DreamMusicPlayerLyric.h"
#include "DreamLyricAssetFactory.h"
#include "DreamLyricAsset.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "EditorAssetLibrary.h"
#include "UObject/SavePackage.h"

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerLyricModule"

static const FName ImportLyricFileMenuName = TEXT("ImportLyricFile");

void FDreamMusicPlayerLyricModule::StartupModule()
{
	// 确保工厂类被注册
	// UFactory 类会被自动发现，但我们可以在这里进行一些初始化
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerLyric module started"));
	
	// 注意：不在这里加载第三方 DLL，而是在实际使用时才加载
	// 这样可以避免模块启动时因为 DLL 找不到而失败
	
	// 注册菜单（仅在编辑器环境下）
#if WITH_EDITOR
	RegisterMenus();
#endif
}

void FDreamMusicPlayerLyricModule::ShutdownModule()
{
#if WITH_EDITOR
	UnregisterMenus();
#endif
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerLyric module shutdown"));
}

void FDreamMusicPlayerLyricModule::RegisterMenus()
{
	// 获取工具菜单
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	// 扩展主菜单栏的"工具"菜单
	FToolMenuOwner Owner = FToolMenuOwner(this);
	UToolMenu* Menu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools");
	
	// 添加分隔符和菜单项
	FToolMenuSection& Section = Menu->AddSection("DreamMusicPlayerLyric", LOCTEXT("DreamMusicPlayerLyricSection", "Dream Music Player Lyric"));
	Section.AddMenuEntry(
		ImportLyricFileMenuName,
		LOCTEXT("ImportLyricFile", "Import Lyric File"),
		LOCTEXT("ImportLyricFileTooltip", "Import a lyric file (LRC, ASS, or SRT) as a Lyric Asset"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
		FUIAction(FExecuteAction::CreateRaw(this, &FDreamMusicPlayerLyricModule::OnImportLyricFileClicked))
	);
}

void FDreamMusicPlayerLyricModule::UnregisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (ToolMenus)
	{
		ToolMenus->RemoveSection("LevelEditor.MainMenu.Tools", "DreamMusicPlayerLyric");
	}
}

void FDreamMusicPlayerLyricModule::OnImportLyricFileClicked()
{
	// 打开文件选择对话框
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get DesktopPlatform"));
		return;
	}

	// 设置文件类型过滤器
	const FString FileTypes = TEXT("Lyric Files (*.lrc;*.ass;*.srt)|*.lrc;*.ass;*.srt|LRC Files (*.lrc)|*.lrc|ASS Files (*.ass)|*.ass|SRT Files (*.srt)|*.srt|All Files (*.*)|*.*");
	
	TArray<FString> OutFilenames;
	uint32 SelectionFlag = 0; // 单选

	// 打开文件选择对话框
	bool bFileSelected = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		LOCTEXT("ImportLyricFileDialogTitle", "Import Lyric File").ToString(),
		FPaths::ProjectContentDir(),
		TEXT(""),
		FileTypes,
		SelectionFlag,
		OutFilenames
	);

	if (!bFileSelected || OutFilenames.Num() == 0)
	{
		// 用户取消了选择
		return;
	}

	FString SelectedFile = OutFilenames[0];
	if (!FPaths::FileExists(SelectedFile))
	{
		UE_LOG(LogTemp, Error, TEXT("Selected file does not exist: %s"), *SelectedFile);
		return;
	}

	// 获取目标路径（Content Browser 的当前路径，或默认路径）
	FString TargetPath = TEXT("/Game");
	
	IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedPaths;
	ContentBrowser.GetSelectedPathViewFolders(SelectedPaths);
	if (SelectedPaths.Num() > 0)
	{
		TargetPath = SelectedPaths[0];
		// 确保路径在 Content 目录下
		if (!TargetPath.StartsWith(TEXT("/Game")))
		{
			TargetPath = TEXT("/Game");
		}
	}

	// 使用 AssetTools 模块来导入资产（更安全的方式，避免堆损坏）
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	
	// 创建工厂实例
	ULyricAssetFactory* Factory = NewObject<ULyricAssetFactory>();
	if (!Factory)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create LyricAssetFactory"));
		return;
	}
	
	// 创建导入任务
	TArray<FString> FilesToImport;
	FilesToImport.Add(SelectedFile);
	
	// 导入资产（ImportAssets 会返回导入的对象数组）
	TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssets(
		FilesToImport,
		TargetPath,
		Factory,
		true,  // bSyncToBrowser - 自动同步到 Content Browser
		nullptr,  // FilesAndDestinations
		false,  // bAllowAsyncImport
		false   // bSceneImport
	);
	
	if (ImportedObjects.Num() > 0)
	{
		UDreamLyricAsset* Asset = Cast<UDreamLyricAsset>(ImportedObjects[0]);
		if (Asset)
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully imported lyric asset: %s"), *Asset->GetPathName());
			
			// 在 Content Browser 中选择新创建的资产
			TArray<UObject*> AssetsToSelect;
			AssetsToSelect.Add(Asset);
			ContentBrowser.SyncBrowserToAssets(AssetsToSelect);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to cast imported object to ULyricAsset"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to import lyric file: %s"), *SelectedFile);
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FDreamMusicPlayerLyricModule, DreamMusicPlayerLyric)