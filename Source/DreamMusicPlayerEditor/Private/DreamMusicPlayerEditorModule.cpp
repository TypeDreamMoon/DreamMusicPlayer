#include "DreamMusicPlayerEditorModule.h"
#include "LyricModule/DreamLyricAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "DreamMusicPlayerEditorStyles.h"
#include "IAssetTools.h"

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
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerEditorModule, DreamMusicPlayerEditor)
