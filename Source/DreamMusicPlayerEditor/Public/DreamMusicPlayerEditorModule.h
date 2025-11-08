#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDreamLyricAssetTypeActions;

/**
 * @brief Dream Music Player 编辑器模块
 */
class FDreamMusicPlayerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
	/** 资产类型操作数组 */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> CreatedAssetTypeActions;
};
