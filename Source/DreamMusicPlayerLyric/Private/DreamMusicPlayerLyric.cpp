#include "DreamMusicPlayerLyric.h"
#include "DreamLyricAsset.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerLyricModule"


void FDreamMusicPlayerLyricModule::StartupModule()
{
}

void FDreamMusicPlayerLyricModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDreamMusicPlayerLyricModule, DreamMusicPlayerLyric)
