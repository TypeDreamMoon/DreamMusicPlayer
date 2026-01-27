#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDreamMusicPlayerImporterModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void UnregisterMenus();
	void OnImportLyricFileClicked();
	void OnImportMusicFileClicked();
};
