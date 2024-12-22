#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DreamMusicPlayerCommon.generated.h"

class UDreamMusicData;
class UConstantQNRT;
class ULoudnessNRT;

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayState : uint8
{
	EDMPPS_Stop			=	0 UMETA(DisplayName = "Stop"),
	EDMPPS_Playing		=	1 UMETA(DisplayName = "Playing"),
	EDMPPS_Paused		=	2 UMETA(DisplayName = "Paused"),
};

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayMode : uint8
{
	EDMPPS_Normal		=	0 UMETA(DisplayName = "Normal"),
	EDMPPS_Loop			=	1 UMETA(DisplayName = "Loop"),
	EDMPPS_Random		=	2 UMETA(DisplayName = "Random")
};

USTRUCT(BlueprintType)
struct FDreamMusicLyricTimestamp
{
	GENERATED_BODY()

public:
	FDreamMusicLyricTimestamp() : Minute(0), Seconds(0), Millisecond(0)
	{
	};

	FDreamMusicLyricTimestamp(int InMinute, int InSeconds, int InMillisecond) :
		Minute(InMinute), Seconds(InSeconds), Millisecond(InMillisecond)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Minute;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Seconds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Millisecond;

public:
	bool operator==(const FDreamMusicLyricTimestamp& Target) const;
	bool operator>=(const FDreamMusicLyricTimestamp& Target) const;
	bool operator>(const FDreamMusicLyricTimestamp& Target) const;
	bool operator<=(const FDreamMusicLyricTimestamp& Target) const;
	bool operator<(const FDreamMusicLyricTimestamp& Target) const;
	int TotalMilliseconds() const
	{
		return Minute * 60000 + Seconds * 1000 + Millisecond;
	}
};

USTRUCT(BlueprintType)
struct FDreamMusicLyric
{
	GENERATED_BODY()

public:
	FDreamMusicLyric()
	{
	};
	FDreamMusicLyric(FString Line);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp Timestamp = FDreamMusicLyricTimestamp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Translate;

public:
	bool operator==(const FDreamMusicLyric& Target) const;
	bool operator==(const FDreamMusicLyricTimestamp& Target) const;
	bool operator!=(const FDreamMusicLyric& Target) const;
	bool IsNotEmpty() const
	{
		return Timestamp.TotalMilliseconds() > 0 || !Content.IsEmpty() || !Translate.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct FDreamMusicInformation
{
	GENERATED_BODY()

public:
	FDreamMusicInformation()
	{
	};

	FDreamMusicInformation(FString InTitle, FString InArtist, FString InAlbum, TObjectPtr<UTexture2D> InCover,
	                      FString InGenre) :
		Title(InTitle), Artist(InArtist), Album(InAlbum), Cover(InCover),
		Genre(InGenre)
	{
	}

public:
	//歌曲标题
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title;

	//歌曲作者
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Artist;

	//歌曲专辑
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Album;

	//歌曲封面
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Cover;

	//歌曲流派
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Genre;
public:
	bool IsValid() const;
	bool operator==(const FDreamMusicInformation& Target) const;
};

// 歌曲数据
USTRUCT(BlueprintType)
struct FDreamMusicInformationData
{
	GENERATED_BODY()

public:
	FDreamMusicInformationData()
	{
	};
public:
	// 音乐资源软对象引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Music;

	// 内容路径请在ProjectSetting -> DreamPlugins -> Musicplayer -> LyricContentPath 中配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(GetOptions = "DreamMusicPlayer.DreamMusicPlayerBlueprint.GetLyricFileNames"))
	FString LyricFileName;

	// 频谱可视化对象
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MetaClass = "ConstantQNRT"))
	FSoftObjectPath ConstantQ;

	// 响度可视化对象
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MetaClass = "LoudnessNRT"))
	FSoftObjectPath Loudness;
public:
	bool IsValid() const;
	bool operator==(const FDreamMusicInformationData& Target) const;
};

// 歌曲数据表
USTRUCT(BlueprintType)
struct FDreamMusicDataStruct : public FTableRowBase
{
	GENERATED_BODY()
public:
	FDreamMusicDataStruct() : Information(FDreamMusicInformation()), Data(FDreamMusicInformationData()) {};
	FDreamMusicDataStruct(FDreamMusicInformation InInformation, FDreamMusicInformationData InfomationData)
		: Information(InInformation), Data(InfomationData) {};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformation Information;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformationData Data;

public:
	bool IsVaild() const;
	bool operator==(const FDreamMusicDataStruct& Target) const;
};
// 歌曲数据表
USTRUCT(BlueprintType)
struct FDreamMusicPlayerSondList : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDreamMusicData* MusicData;
};

USTRUCT(BlueprintType)
struct FDreamMusicPlayerFadeAudioSetting
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableFadeAudio = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutDuration = 0.5f;
};
