#include "DreamLyricAssetTypeActions.h"
#include "DreamLyricAsset.h"
#include "DreamLyricAssetEditor.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "DreamLyricAssetTypeActions"

FText FDreamLyricAssetTypeActions::GetName() const
{
	return LOCTEXT("FDreamLyricAssetTypeActionsName", "Dream Lyric Asset");
}

FColor FDreamLyricAssetTypeActions::GetTypeColor() const
{
	return FColor(200, 100, 200); // 紫色
}

UClass* FDreamLyricAssetTypeActions::GetSupportedClass() const
{
	return UDreamLyricAsset::StaticClass();
}

void FDreamLyricAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UDreamLyricAsset* LyricAsset = Cast<UDreamLyricAsset>(Obj))
		{
			TSharedRef<FDreamLyricAssetEditor> NewEditor(new FDreamLyricAssetEditor());
			NewEditor->InitLyricAssetEditor(Mode, EditWithinLevelEditor, LyricAsset);
		}
	}
}

uint32 FDreamLyricAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& FDreamLyricAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus;
	return SubMenus;
}

#undef LOCTEXT_NAMESPACE

