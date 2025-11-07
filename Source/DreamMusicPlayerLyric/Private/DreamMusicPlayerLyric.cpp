#include "DreamMusicPlayerLyric.h"
#include "LyricAssetFactory.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FDreamMusicPlayerLyricModule"

void FDreamMusicPlayerLyricModule::StartupModule()
{
	// 确保工厂类被注册
	// UFactory 类会被自动发现，但我们可以在这里进行一些初始化
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerLyric module started"));
	
	// 注意：不在这里加载第三方 DLL，而是在实际使用时才加载
	// 这样可以避免模块启动时因为 DLL 找不到而失败
}

void FDreamMusicPlayerLyricModule::ShutdownModule()
{
	UE_LOG(LogTemp, Log, TEXT("DreamMusicPlayerLyric module shutdown"));
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FDreamMusicPlayerLyricModule, DreamMusicPlayerLyric)