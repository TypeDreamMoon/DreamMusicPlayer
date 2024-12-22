// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Engine/DataAsset.h"
#include "DreamMusicData.generated.h"

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicDataStruct Data;
};
