#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include <chrono>
#include "Kismet/KismetStringLibrary.h"
#include "DreamMusicPlayerCommon.generated.h"

class UDreamMusicPlayerExpansionData;
class UDreamMusicData;
class UConstantQNRT;
class ULoudnessNRT;

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayState : uint8
{
	EDMPPS_Stop = 0 UMETA(DisplayName = "Stop"),
	EDMPPS_Playing = 1 UMETA(DisplayName = "Playing"),
	EDMPPS_Paused = 2 UMETA(DisplayName = "Paused"),
};

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayMode : uint8
{
	EDMPPS_Normal = 0 UMETA(DisplayName = "Normal"),
	EDMPPS_Loop = 1 UMETA(DisplayName = "Loop"),
	EDMPPS_Random = 2 UMETA(DisplayName = "Random")
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

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicInformationData& Target) const;
};

// 歌曲数据表
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicDataStruct
{
	GENERATED_BODY()

public:
	FDreamMusicDataStruct() : Information(FDreamMusicInformation()), Data(FDreamMusicInformationData())
	{
	};

	FDreamMusicDataStruct(FDreamMusicInformation InInformation, FDreamMusicInformationData InfomationData)
		: Information(InInformation), Data(InfomationData)
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformation Information;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformationData Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UDreamMusicPlayerExpansionData*> ExpansionDatas;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicDataStruct& Target) const;

	bool HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const;

	template <typename T>
	T* GetExpansionData() const
	{
		for (auto ExpansionData : ExpansionDatas)
		{
			if (auto CastedExpansionData = Cast<T>(ExpansionData))
			{
				return CastedExpansionData;
			}
		}
		return nullptr;
	}
};

// 歌曲数据表
USTRUCT(BlueprintType)
struct FDreamMusicPlayerSongList : public FTableRowBase
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
