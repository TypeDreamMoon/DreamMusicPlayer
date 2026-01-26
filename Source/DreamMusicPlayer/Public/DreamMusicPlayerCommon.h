#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include <chrono>
#include "Kismet/KismetStringLibrary.h"
#include "DreamMusicPlayerCommon.generated.h"

class UDreamMusicPlayerExpansionData;
class UDreamMusicDataAsset;
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

// 歌曲数据表
USTRUCT(BlueprintType)
struct FDreamMusicPlayerSongList : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDreamMusicDataAsset* MusicData;
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
