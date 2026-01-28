#pragma once

#include "CoreMinimal.h"

/**
 * 负责在编辑器中注册歌词窗口的管理器
 * 这是一个静态工具类，用于处理编辑器的 Tab 注册逻辑
 */
class FLyricsWindowManager
{
public:
	// 注册窗口 (在模块启动时调用)
	static void RegisterTabSpawner();

	// 注销窗口 (在模块关闭时调用)
	static void UnregisterTabSpawner();

	// 打开窗口的命令 (可选，用于快捷键绑定等)
	static void InvokeTab();

private:
	// Tab 的唯一 ID
	static const FName LyricsTabId;
};