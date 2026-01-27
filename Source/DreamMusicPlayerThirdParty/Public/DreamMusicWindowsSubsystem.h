// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StylusInputWintab/Private/WintabMessageHandler.h"
#include "DreamMusicWindowsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMTCPlay);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMTCPause);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMTCNext);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMTCPrevious);

DECLARE_MULTICAST_DELEGATE(FOnSMTCPlayNative);
DECLARE_MULTICAST_DELEGATE(FOnSMTCPauseNative);
DECLARE_MULTICAST_DELEGATE(FOnSMTCNextNative);
DECLARE_MULTICAST_DELEGATE(FOnSMTCPreviousNative);

UENUM(BlueprintType)
enum class ESMTCPlaybackStatus : uint8
{
	Closed = 0,
	Opened,
	Changing,
	Stopped,
	Playing,
	Paused
};

UCLASS(DisplayName = "Windows Subsystem")
class DREAMMUSICPLAYERTHIRDPARTY_API UDreamMusicWindowsSubsystem : public UGameInstanceSubsystem, public IWindowsMessageHandler
{
	GENERATED_BODY()

public:
	// Subsystem 初始化与清理
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- 功能接口 ---

	/** * 初始化 SMTC。
	 * 建议在第一个关卡的 BeginPlay 或 GameInstance 的 Init 中调用，确保窗口句柄已创建。
	 */
	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	bool InitializeSMTC();

	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	void UpdatePlaybackStatus(ESMTCPlaybackStatus NewStatus);

	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	void UpdateMetadata(const FString& Title, const FString& Artist);

	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	void UpdateButtons(bool bEnablePlay, bool bEnablePause, bool bEnableNext, bool bEnablePrevious);

	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	void UpdateThumbnailAsync(const FString& AbsolutePath);

	UFUNCTION(BlueprintCallable, Category = "Windows SMTC")
	void UpdateThumbnailFromTexture(UTexture2D* Texture);

	/**
	 * 初始化任务栏按钮
	 * 必须在窗口显示后调用。
	 */
	UFUNCTION(BlueprintCallable, Category = "Windows Taskbar")
	bool InitializeTaskbarButtons();

	/**
	 * 设置任务栏按钮的图标 (.ico 文件)
	 * @param PlayIconPath 播放图标路径
	 * @param PauseIconPath 暂停图标路径
	 * @param NextIconPath 下一曲图标路径
	 * @param PrevIconPath 上一曲图标路径
	 */
	UFUNCTION(BlueprintCallable, Category = "Windows Taskbar")
	void SetTaskbarIcons(const FString& PlayIconPath, const FString& PauseIconPath, const FString& NextIconPath, const FString& PrevIconPath);

	// --- 事件 ---

	UPROPERTY(BlueprintAssignable, Category = "Windows SMTC")
	FOnSMTCPlay OnPlayPressed;

	UPROPERTY(BlueprintAssignable, Category = "Windows SMTC")
	FOnSMTCPause OnPausePressed;

	UPROPERTY(BlueprintAssignable, Category = "Windows SMTC")
	FOnSMTCNext OnNextPressed;

	UPROPERTY(BlueprintAssignable, Category = "Windows SMTC")
	FOnSMTCPrevious OnPreviousPressed;

	// C++ Native Delegates
	FOnSMTCPlayNative OnPlayPressedNative;
	FOnSMTCPauseNative OnPausePressedNative;
	FOnSMTCNextNative OnNextPressedNative;
	FOnSMTCPreviousNative OnPreviousPressedNative;

protected:
#if PLATFORM_WINDOWS
	// IWindowsMessageHandler 接口实现
	virtual bool ProcessMessage(HWND Hwnd, uint32 Message, WPARAM wParam, LPARAM lParam, int32& OutResult) override;
#endif

private:
#if PLATFORM_WINDOWS
	void* SMTCInstance = nullptr;
	int64 ButtonPressedToken = 0;
	void HandleButtonPressed(int ButtonID);
	void SetThumbnailInternal(const FString& FilePath);

	// Taskbar 成员
	void* TaskbarList3 = nullptr; // ITaskbarList3*
	bool bTaskbarInitialized = false;
	Windows::HICON IconPlay = nullptr;
	Windows::HICON IconPause = nullptr;
	Windows::HICON IconNext = nullptr;
	Windows::HICON IconPrev = nullptr;
	ESMTCPlaybackStatus CurrentStatus = ESMTCPlaybackStatus::Stopped;
	// 辅助函数
	void UpdateTaskbarButtonStates();
	Windows::HICON LoadIconFromPath(const FString& Path);
#endif
};
