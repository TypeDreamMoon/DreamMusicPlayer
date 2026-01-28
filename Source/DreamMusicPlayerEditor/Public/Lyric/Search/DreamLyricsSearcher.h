#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"

// 音乐平台枚举
enum class ELyricsPlatform
{
    Netease,    // 网易云音乐 (EAPI Client)
    Lrclib,     // Lrclib (Open Source)
    QQ,         // QQ音乐 (Reserved)
};

// 歌曲基本信息
struct FSongInfo
{
    FString ID;
    FString Title;
    FString Artist;
    FString Album;
    int32 Duration; // ms
    FString Source;
};

// 歌词详细信息
struct FLyricResult
{
    FString SongID;
    FString Lyric;      // 原始内容 (LRC)
    FString TLyric;     // 翻译内容 (如果有)
    FString KLyric;     // 逐字歌词 (如果有 YRC/KRC)
};

// 回调定义
using FOnSearchComplete = TFunction<void(bool bSuccess, const TArray<FSongInfo>& Results)>;
using FOnLyricComplete = TFunction<void(bool bSuccess, const FLyricResult& Result)>;

/**
 * 歌词下载工具类
 */
class FLyricsTool
{
public:
    static void Search(const FString& Keyword, ELyricsPlatform Platform, FOnSearchComplete Callback);
    static void GetLyric(const FString& SongID, ELyricsPlatform Platform, FOnLyricComplete Callback);
};

/**
 * 内部请求处理器
 */
class FLyricsRequestHandler : public TSharedFromThis<FLyricsRequestHandler>
{
public:
    void Search(const FString& Keyword, ELyricsPlatform Platform, FOnSearchComplete Callback);
    void GetLyric(const FString& SongID, ELyricsPlatform Platform, FOnLyricComplete Callback);

private:
    // 登录与 Session 管理
    void CheckLoginAndExecute(TFunction<void()> OnReady);
    
    // Netease 逻辑
    void PerformNeteaseSearch(const FString& Keyword, FOnSearchComplete Callback);
    void PerformNeteaseLyric(const FString& SongID, FOnLyricComplete Callback);

    // Lrclib 逻辑
    void PerformLrclibSearch(const FString& Keyword, FOnSearchComplete Callback);
    void PerformLrclibLyric(const FString& SongID, FOnLyricComplete Callback);

    // 解析器
    TArray<FSongInfo> ParseNeteaseSearch(const FString& Json);
    FLyricResult ParseNeteaseLyric(const FString& Json, const FString& SongID);

    TArray<FSongInfo> ParseLrclibSearch(const FString& Json);
    FLyricResult ParseLrclibLyric(const FString& Json, const FString& SongID);
};