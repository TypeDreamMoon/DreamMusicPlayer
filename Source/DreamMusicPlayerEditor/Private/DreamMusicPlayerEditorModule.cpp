#include "DreamMusicPlayerEditorModule.h"
#include "Lyric/DreamLyricAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "DreamMusicPlayerEditorStyles.h"
#include "IAssetTools.h"
#include "Lyric/Search/LyricsWindowManager.h"

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerEditorModule"

void FDreamMusicPlayerEditorModule::StartupModule()
{
	// 注册资产类型操作
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TSharedPtr<FDreamLyricAssetTypeActions> LyricAssetTypeActions = MakeShareable(new FDreamLyricAssetTypeActions());
	AssetTools.RegisterAssetTypeActions(LyricAssetTypeActions.ToSharedRef());
	CreatedAssetTypeActions.Add(LyricAssetTypeActions);

	FDreamMusicPlayerEditorStyles::Initialize();
	FDreamMusicPlayerEditorStyles::Register();
	
	FLyricsWindowManager::RegisterTabSpawner();
}

void FDreamMusicPlayerEditorModule::ShutdownModule()
{
	// 注销资产类型操作
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& Action : CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();

	FDreamMusicPlayerEditorStyles::Unregister();
	FLyricsWindowManager::UnregisterTabSpawner();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerEditorModule, DreamMusicPlayerEditor)
