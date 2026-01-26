// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DreamMusicPlayerInterfaces.generated.h"

class UDreamMusicPlayerComponent;


UINTERFACE()
class UDreamMusicPlayerCommonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DREAMMUSICPLAYER_API IDreamMusicPlayerCommonInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UDreamMusicPlayerComponent* GetDreamMusicPlayerComponent();
};
