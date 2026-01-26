// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicData.h"
#include "DreamMusicPlayerCommon.h"
#include "Engine/DataAsset.h"
#include "DreamMusicDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicData Data;
};
