#include "Lyric/Search/LyricsWindowManager.h"

#include "Lyric/Search/SLyricsWindow.h"

#if WITH_EDITOR
#include "Widgets/Docking/SDockTab.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#endif

const FName FLyricsWindowManager::LyricsTabId(TEXT("LyricsToolWindow"));

void FLyricsWindowManager::RegisterTabSpawner()
{
#if WITH_EDITOR
	// 1. 定义 Tab 的生成逻辑
	auto SpawnTab = [](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
	{
		return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab) // 游牧类型，可停靠在任何地方
				[
					SNew(SLyricsWindow) // 你的歌词 UI
				];
	};

	// 2. 注册到全局 Tab 管理器
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		                        LyricsTabId,
		                        FOnSpawnTab::CreateLambda(SpawnTab)
	                        )
	                        .SetDisplayName(FText::FromString(TEXT("Lyrics Search"))) // 窗口标题
	                        .SetTooltipText(FText::FromString(TEXT("Open the Lyrics Search Tool")))
	                        .SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory()) // 添加到 "Tools" 分组
	                        .SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Search")); // 设置一个搜索图标
#endif
}

void FLyricsWindowManager::UnregisterTabSpawner()
{
#if WITH_EDITOR
	// 清理注册，防止热重载崩溃
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LyricsTabId);
#endif
}

void FLyricsWindowManager::InvokeTab()
{
#if WITH_EDITOR
	FGlobalTabmanager::Get()->TryInvokeTab(LyricsTabId);
#endif
}
