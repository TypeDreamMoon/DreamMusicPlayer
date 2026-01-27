#include "DreamMusicWindowsSubsystem.h"
#include "Async/Async.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "GenericPlatform/GenericWindow.h"
#include "Widgets/SWindow.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "ImageWriteBlueprintLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "Misc/App.h"

// [修复 1] 引入 SlateApplication 以获取平台应用实例
#include "Framework/Application/SlateApplication.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <shobjidl.h> // ITaskbarList3
#include <shellapi.h>
#include <intrin.h>

// [修复 2] 引入 WindowsApplication 以访问 AddMessageHandler
#include "Windows/WindowsApplication.h"


#ifndef InterlockedCompareExchange
#define InterlockedCompareExchange _InterlockedCompareExchange
#endif
#ifndef InterlockedIncrement
#define InterlockedIncrement _InterlockedIncrement
#endif
#ifndef InterlockedDecrement
#define InterlockedDecrement _InterlockedDecrement
#endif
#ifndef InterlockedExchange
#define InterlockedExchange _InterlockedExchange
#endif

#include <wrl.h>
#include <systemmediatransportcontrolsinterop.h>
#include <windows.media.h>
#include <windows.storage.h>
#include <windows.storage.streams.h>
#include "Windows/HideWindowsPlatformTypes.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::Media;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Storage;
using namespace ABI::Windows::Storage::Streams;

// 定义任务栏按钮的 ID
#define ID_TB_PREV  3001
#define ID_TB_PLAY  3002
#define ID_TB_PAUSE 3003
#define ID_TB_NEXT  3004

#endif

void UDreamMusicWindowsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_WINDOWS
	// [修复 3] 将 GenericApplication 转换为 FWindowsApplication
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<GenericApplication> PlatformApp = FSlateApplication::Get().GetPlatformApplication();
		// 这里的 StaticCast 是安全的，因为我们已经检查了 PLATFORM_WINDOWS
		TSharedPtr<FWindowsApplication> WindowsApp = StaticCastSharedPtr<FWindowsApplication>(PlatformApp);

		if (WindowsApp.IsValid())
		{
			// FWindowsApplication 才有 AddMessageHandler
			WindowsApp->AddMessageHandler(*this);
		}
	}
#endif
}

void UDreamMusicWindowsSubsystem::Deinitialize()
{
#if PLATFORM_WINDOWS
	// [修复 4] 同样在移除时进行转换
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<GenericApplication> PlatformApp = FSlateApplication::Get().GetPlatformApplication();
		TSharedPtr<FWindowsApplication> WindowsApp = StaticCastSharedPtr<FWindowsApplication>(PlatformApp);

		if (WindowsApp.IsValid())
		{
			WindowsApp->RemoveMessageHandler(*this);
		}
	}

	// 清理 SMTC
	if (SMTCInstance)
	{
		ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(SMTCInstance);
		if (ButtonPressedToken != 0) Controls->remove_ButtonPressed({ButtonPressedToken});
		Controls->put_IsEnabled(false);
		Controls = nullptr;
		SMTCInstance = nullptr;
	}

	// 清理 Taskbar
	if (TaskbarList3)
	{
		ComPtr<ITaskbarList3> Taskbar = static_cast<ITaskbarList3*>(TaskbarList3);
		Taskbar = nullptr;
		TaskbarList3 = nullptr;
	}

	// 清理图标
	if (IconPlay) DestroyIcon(IconPlay);
	if (IconPause) DestroyIcon(IconPause);
	if (IconNext) DestroyIcon(IconNext);
	if (IconPrev) DestroyIcon(IconPrev);
#endif
	Super::Deinitialize();
}

// ==========================================
// SMTC 部分 (保持原样，略有精简)
// ==========================================

bool UDreamMusicWindowsSubsystem::InitializeSMTC()
{
#if PLATFORM_WINDOWS
	if (SMTCInstance) return true;

	FString AppId = FApp::GetProjectName();
	SetCurrentProcessExplicitAppUserModelID((PCWSTR)*AppId);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance) return false;
	UGameViewportClient* GameViewport = GameInstance->GetGameViewportClient();
	if (!GameViewport) return false;
	TSharedPtr<SWindow> Window = GameViewport->GetWindow();
	if (!Window.IsValid()) return false;
	HWND Hwnd = (HWND)Window->GetNativeWindow()->GetOSWindowHandle();
	if (!Hwnd) return false;

	ComPtr<ISystemMediaTransportControlsInterop> Interop;
	HRESULT Hr = GetActivationFactory(
		Wrappers::HStringReference(RuntimeClass_Windows_Media_SystemMediaTransportControls).Get(),
		&Interop
	);
	if (FAILED(Hr)) return false;

	ComPtr<ISystemMediaTransportControls> Controls;
	Hr = Interop->GetForWindow(Hwnd, IID_PPV_ARGS(&Controls));
	if (FAILED(Hr) || !Controls) return false;

	Controls.Get()->AddRef();
	SMTCInstance = Controls.Get();
	Controls->put_IsEnabled(true);
	Controls->put_IsPlayEnabled(true);
	Controls->put_IsPauseEnabled(true);
	Controls->put_IsNextEnabled(true);
	Controls->put_IsPreviousEnabled(true);

	auto Handler = Callback<ITypedEventHandler<SystemMediaTransportControls*, SystemMediaTransportControlsButtonPressedEventArgs*>>(
		[this](ISystemMediaTransportControls* Sender, ISystemMediaTransportControlsButtonPressedEventArgs* Args) -> HRESULT
		{
			SystemMediaTransportControlsButton Button;
			if (SUCCEEDED(Args->get_Button(&Button))) this->HandleButtonPressed((int)Button);
			return S_OK;
		});

	EventRegistrationToken Token;
	Hr = Controls->add_ButtonPressed(Handler.Get(), &Token);
	if (SUCCEEDED(Hr)) ButtonPressedToken = Token.value;

	return true;
#endif
	return false;
}

void UDreamMusicWindowsSubsystem::UpdatePlaybackStatus(ESMTCPlaybackStatus NewStatus)
{
#if PLATFORM_WINDOWS
	CurrentStatus = NewStatus; // 记录状态用于 Taskbar 更新

	// 更新 SMTC
	if (SMTCInstance)
	{
		ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(SMTCInstance);
		MediaPlaybackStatus Status = MediaPlaybackStatus::MediaPlaybackStatus_Closed;
		switch (NewStatus)
		{
		case ESMTCPlaybackStatus::Playing: Status = MediaPlaybackStatus::MediaPlaybackStatus_Playing;
			break;
		case ESMTCPlaybackStatus::Paused: Status = MediaPlaybackStatus::MediaPlaybackStatus_Paused;
			break;
		case ESMTCPlaybackStatus::Stopped: Status = MediaPlaybackStatus::MediaPlaybackStatus_Stopped;
			break;
		default: break;
		}
		Controls->put_PlaybackStatus(Status);
	}

	// 更新 Taskbar 按钮 (切换播放/暂停图标)
	UpdateTaskbarButtonStates();
#endif
}

void UDreamMusicWindowsSubsystem::UpdateMetadata(const FString& Title, const FString& Artist)
{
#if PLATFORM_WINDOWS
	if (!SMTCInstance) return;
	ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(SMTCInstance);
	ComPtr<ISystemMediaTransportControlsDisplayUpdater> Updater;
	if (SUCCEEDED(Controls->get_DisplayUpdater(&Updater)))
	{
		Updater->put_Type(MediaPlaybackType::MediaPlaybackType_Music);
		ComPtr<IMusicDisplayProperties> MusicProps;
		if (SUCCEEDED(Updater->get_MusicProperties(&MusicProps)))
		{
			Wrappers::HString HTitle;
			HTitle.Set(*Title);
			MusicProps->put_Title(HTitle.Get());
			Wrappers::HString HArtist;
			HArtist.Set(*Artist);
			MusicProps->put_Artist(HArtist.Get());
			Updater->Update();
		}
	}
#endif
}

void UDreamMusicWindowsSubsystem::UpdateThumbnailAsync(const FString& AbsolutePath)
{
#if PLATFORM_WINDOWS
	FString Path = AbsolutePath;
	FPaths::MakePlatformFilename(Path);
	Async(EAsyncExecution::ThreadPool, [this, Path]()
	{
		if (!FPaths::FileExists(Path)) return;
		AsyncTask(ENamedThreads::GameThread, [this, Path]() { if (IsValid(this)) SetThumbnailInternal(Path); });
	});
#endif
}

void UDreamMusicWindowsSubsystem::UpdateThumbnailFromTexture(UTexture2D* Texture)
{
#if PLATFORM_WINDOWS
	if (!Texture) return;
	FString TempDir = FPaths::ProjectSavedDir() / TEXT("Temp/SMTC");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*TempDir)) PlatformFile.CreateDirectoryTree(*TempDir);

	FString TempFilename = FString::Printf(TEXT("Cover_%s.jpg"), *FGuid::NewGuid().ToString());
	FString FullPath = TempDir / TempFilename;
	FullPath = FPaths::ConvertRelativePathToFull(FullPath);
	FPaths::MakePlatformFilename(FullPath);

	FImageWriteOptions Options;
	Options.bAsync = true;
	Options.bOverwriteFile = true;
	Options.Format = EDesiredImageFormat::JPG;
	Options.CompressionQuality = 90;
	Options.NativeOnComplete = [FullPath, this](bool bSuccessful) { if (bSuccessful) UpdateThumbnailAsync(FullPath); };

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) return;
	FVector2D Size(Texture->GetSizeX(), Texture->GetSizeY());
	UTextureRenderTarget2D* RenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(World, Size.X, Size.Y, ETextureRenderTargetFormat::RTF_RGBA8_SRGB);
	if (RenderTarget)
	{
		UCanvas* CanvasObj;
		FDrawToRenderTargetContext Context;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, RenderTarget, CanvasObj, Size, Context);
		if (CanvasObj) CanvasObj->K2_DrawTexture(Texture, FVector2D::ZeroVector, Size, FVector2D::ZeroVector);
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(World, Context);
		UImageWriteBlueprintLibrary::ExportToDisk(RenderTarget, FullPath, Options);
	}
#endif
}

#if PLATFORM_WINDOWS
void UDreamMusicWindowsSubsystem::SetThumbnailInternal(const FString& FilePath)
{
	if (!SMTCInstance) return;
	ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(SMTCInstance);

	// WinRT Logic ... (保持之前修复后的版本)
	ComPtr<IStorageFileStatics> StorageFileStatics;
	if (FAILED(GetActivationFactory(Wrappers::HStringReference(RuntimeClass_Windows_Storage_StorageFile).Get(), &StorageFileStatics))) return;

	FString WindowsPath = FilePath;
	FPaths::MakePlatformFilename(WindowsPath);
	Wrappers::HString HPath;
	HPath.Set(*WindowsPath);

	ComPtr<IAsyncOperation<StorageFile*>> AsyncOp;
	if (FAILED(StorageFileStatics->GetFileFromPathAsync(HPath.Get(), &AsyncOp))) return;

	auto CallbackHandler = Callback<IAsyncOperationCompletedHandler<StorageFile*>>(
		[this](IAsyncOperation<StorageFile*>* Op, AsyncStatus Status) -> HRESULT
		{
			if (Status != AsyncStatus::Completed) return S_OK;
			ComPtr<IStorageFile> File;
			if (FAILED(Op->GetResults(&File)) || !File) return S_OK;
			ComPtr<IRandomAccessStreamReferenceStatics> StreamRefStatics;
			if (FAILED(GetActivationFactory(Wrappers::HStringReference(RuntimeClass_Windows_Storage_Streams_RandomAccessStreamReference).Get(), &StreamRefStatics))) return S_OK;
			ComPtr<IRandomAccessStreamReference> StreamRef;
			if (FAILED(StreamRefStatics->CreateFromFile(File.Get(), &StreamRef))) return S_OK;

			if (this->SMTCInstance)
			{
				ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(this->SMTCInstance);
				ComPtr<ISystemMediaTransportControlsDisplayUpdater> Updater;
				if (SUCCEEDED(Controls->get_DisplayUpdater(&Updater)))
				{
					Updater->put_Thumbnail(StreamRef.Get());
					Updater->Update();
				}
			}
			return S_OK;
		});
	AsyncOp->put_Completed(CallbackHandler.Get());
}
#endif

void UDreamMusicWindowsSubsystem::UpdateButtons(bool bEnablePlay, bool bEnablePause, bool bEnableNext, bool bEnablePrevious)
{
	// 这个函数主要用于 SMTC，Taskbar 按钮通过 UpdateTaskbarButtonStates 管理
#if PLATFORM_WINDOWS
	if (!SMTCInstance) return;
	ComPtr<ISystemMediaTransportControls> Controls = static_cast<ISystemMediaTransportControls*>(SMTCInstance);
	Controls->put_IsPlayEnabled(bEnablePlay);
	Controls->put_IsPauseEnabled(bEnablePause);
	Controls->put_IsNextEnabled(bEnableNext);
	Controls->put_IsPreviousEnabled(bEnablePrevious);
#endif
}

void UDreamMusicWindowsSubsystem::HandleButtonPressed(int ButtonID)
{
	// 处理 SMTC 按钮事件
	AsyncTask(ENamedThreads::GameThread, [this, ButtonID]()
	{
		if (!IsValid(this)) return;
		switch (ButtonID)
		{
		case 0: OnPlayPressed.Broadcast();
			OnPlayPressedNative.Broadcast();
			break;
		case 1: OnPausePressed.Broadcast();
			OnPausePressedNative.Broadcast();
			break;
		case 5: OnNextPressed.Broadcast();
			OnNextPressedNative.Broadcast();
			break;
		case 6: OnPreviousPressed.Broadcast();
			OnPreviousPressedNative.Broadcast();
			break;
		}
	});
}

// ==========================================
// Taskbar Thumbnail Toolbar 部分 (新增)
// ==========================================

bool UDreamMusicWindowsSubsystem::InitializeTaskbarButtons()
{
#if PLATFORM_WINDOWS
	if (bTaskbarInitialized) return true;

	// 1. 获取窗口句柄
	if (!GEngine || !GEngine->GameViewport) return false;
	TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
	if (!Window.IsValid()) return false;
	HWND Hwnd = (HWND)Window->GetNativeWindow()->GetOSWindowHandle();

	// 2. 创建 ITaskbarList3 接口
	ComPtr<ITaskbarList3> Taskbar;
	HRESULT Hr = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Taskbar));
	if (FAILED(Hr)) return false;

	Hr = Taskbar->HrInit();
	if (FAILED(Hr)) return false;

	// 保存指针 (AddRef 已经在 CoCreateInstance 中完成，但我们用 ComPtr 管理，这里提取 raw pointer 手动管理或 detach)
	// 为了简单起见，我们这里 AddRef 存入 void*，并在 Deinitialize 释放
	Taskbar.Get()->AddRef();
	TaskbarList3 = Taskbar.Get();

	// 3. 准备图标
	// 如果没有通过 SetTaskbarIcons 设置图标，尝试加载系统默认图标作为 Fallback
	// 注意：SHGetStockIconInfo 仅 Win Vista+，这里为了兼容性简单使用 LoadIcon
	if (!IconPrev) IconPrev = LoadIcon(NULL, IDI_APPLICATION); // 占位符
	if (!IconPlay) IconPlay = LoadIcon(NULL, IDI_SHIELD); // 占位符
	if (!IconPause) IconPause = LoadIcon(NULL, IDI_WARNING); // 占位符
	if (!IconNext) IconNext = LoadIcon(NULL, IDI_APPLICATION); // 占位符

	// 4. 定义按钮
	THUMBBUTTON Buttons[3];

	// 上一曲
	Buttons[0].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
	Buttons[0].iId = ID_TB_PREV;
	Buttons[0].iBitmap = 0;
	Buttons[0].hIcon = IconPrev;
	wcscpy_s(Buttons[0].szTip, L"Previous");
	Buttons[0].dwFlags = THBF_ENABLED;

	// 播放/暂停 (初始)
	Buttons[1].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
	Buttons[1].iId = ID_TB_PLAY; // 初始 ID
	Buttons[1].iBitmap = 0;
	Buttons[1].hIcon = IconPlay;
	wcscpy_s(Buttons[1].szTip, L"Play");
	Buttons[1].dwFlags = THBF_ENABLED;

	// 下一曲
	Buttons[2].dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;
	Buttons[2].iId = ID_TB_NEXT;
	Buttons[2].iBitmap = 0;
	Buttons[2].hIcon = IconNext;
	wcscpy_s(Buttons[2].szTip, L"Next");
	Buttons[2].dwFlags = THBF_ENABLED;

	// 5. 添加按钮到任务栏
	// 注意：必须在窗口可见后调用
	Hr = Taskbar->ThumbBarAddButtons(Hwnd, 3, Buttons);

	if (SUCCEEDED(Hr))
	{
		bTaskbarInitialized = true;
		UpdateTaskbarButtonStates(); // 设置初始状态
		return true;
	}

#endif
	return false;
}

void UDreamMusicWindowsSubsystem::SetTaskbarIcons(const FString& PlayIconPath, const FString& PauseIconPath, const FString& NextIconPath, const FString& PrevIconPath)
{
#if PLATFORM_WINDOWS
	// 释放旧图标
	if (IconPlay) DestroyIcon(IconPlay);
	if (IconPause) DestroyIcon(IconPause);
	if (IconNext) DestroyIcon(IconNext);
	if (IconPrev) DestroyIcon(IconPrev);

	IconPlay = LoadIconFromPath(PlayIconPath);
	IconPause = LoadIconFromPath(PauseIconPath);
	IconNext = LoadIconFromPath(NextIconPath);
	IconPrev = LoadIconFromPath(PrevIconPath);

	// 如果已经初始化，刷新按钮
	if (bTaskbarInitialized)
	{
		UpdateTaskbarButtonStates();
	}
#endif
}

#if PLATFORM_WINDOWS
HICON UDreamMusicWindowsSubsystem::LoadIconFromPath(const FString& Path)
{
	if (Path.IsEmpty() || !FPaths::FileExists(Path)) return nullptr;

	FString WinPath = Path;
	FPaths::MakePlatformFilename(WinPath);

	// 加载 16x16 或 32x32 图标
	return (HICON)LoadImage(NULL, *WinPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
}

void UDreamMusicWindowsSubsystem::UpdateTaskbarButtonStates()
{
	if (!bTaskbarInitialized || !TaskbarList3) return;

	if (!GEngine || !GEngine->GameViewport) return;
	TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
	if (!Window.IsValid()) return;
	HWND Hwnd = (HWND)Window->GetNativeWindow()->GetOSWindowHandle();

	ComPtr<ITaskbarList3> Taskbar = static_cast<ITaskbarList3*>(TaskbarList3);

	THUMBBUTTON Button;
	Button.dwMask = THB_BITMAP | THB_TOOLTIP | THB_FLAGS;

	// 更新中间的播放/暂停按钮
	if (CurrentStatus == ESMTCPlaybackStatus::Playing)
	{
		// 显示暂停按钮
		Button.iId = ID_TB_PAUSE; // 按钮 ID 必须匹配消息处理
		Button.hIcon = IconPause;
		wcscpy_s(Button.szTip, L"Pause");
	}
	else
	{
		// 显示播放按钮
		Button.iId = ID_TB_PLAY;
		Button.hIcon = IconPlay;
		wcscpy_s(Button.szTip, L"Play");
	}
	Button.dwFlags = THBF_ENABLED;

	// ITaskbarList3::ThumbBarUpdateButtons 用于更新现有按钮
	// 注意：我们只更新索引为 1 的按钮（中间那个）
	// 但 API 是按 ID 更新的，所以我们需要确保 AddButtons 时 ID 正确
	// 实际上 ThumbBarUpdateButtons 需要传入数组。

	// 重新构建所有按钮状态
	THUMBBUTTON Buttons[3];

	// Prev
	Buttons[0].iId = ID_TB_PREV;
	Buttons[0].hIcon = IconPrev;
	Buttons[0].dwFlags = THBF_ENABLED;
	Buttons[0].dwMask = THB_ICON | THB_FLAGS; // 只更新图标和状态

	// Play/Pause
	Buttons[1].iId = (CurrentStatus == ESMTCPlaybackStatus::Playing) ? ID_TB_PAUSE : ID_TB_PLAY;
	Buttons[1].hIcon = (CurrentStatus == ESMTCPlaybackStatus::Playing) ? IconPause : IconPlay;
	Buttons[1].dwFlags = THBF_ENABLED;
	Buttons[1].dwMask = THB_ICON | THB_TOOLTIP | THB_FLAGS;
	wcscpy_s(Buttons[1].szTip, (CurrentStatus == ESMTCPlaybackStatus::Playing) ? L"Pause" : L"Play");

	// Next
	Buttons[2].iId = ID_TB_NEXT;
	Buttons[2].hIcon = IconNext;
	Buttons[2].dwFlags = THBF_ENABLED;
	Buttons[2].dwMask = THB_ICON | THB_FLAGS;

	Taskbar->ThumbBarUpdateButtons(Hwnd, 3, Buttons);
}

// 消息处理核心
bool UDreamMusicWindowsSubsystem::ProcessMessage(HWND Hwnd, uint32 Message, WPARAM wParam, LPARAM lParam, int32& OutResult)
{
	// 监听 WM_COMMAND 消息
	if (Message == WM_COMMAND)
	{
		// HIWORD(wParam) 表示通知码，对于任务栏按钮，它是 THBN_CLICKED
		if (HIWORD(wParam) == THBN_CLICKED)
		{
			int ButtonID = LOWORD(wParam);

			// 切换回 GameThread 执行逻辑
			HandleButtonPressed(ButtonID); // 这里复用了之前 SMTC 的逻辑，稍微修改一下映射即可

			switch (ButtonID)
			{
			case ID_TB_PREV:
				HandleButtonPressed(6); // 6 = Previous
				break;
			case ID_TB_PLAY:
				HandleButtonPressed(0); // 0 = Play
				break;
			case ID_TB_PAUSE:
				HandleButtonPressed(1); // 1 = Pause
				break;
			case ID_TB_NEXT:
				HandleButtonPressed(5); // 5 = Next
				break;
			}
			return true; // 已处理
		}
	}
	return false; // 未处理，交给其他处理器
}
#endif
