// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicAudioManager.h"
#include "DreamMusicAudioManager_Fade.generated.h"

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicAudioManager_Fade : public UDreamMusicAudioManager
{
	GENERATED_BODY()

public:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;
	virtual void Deinitialize() override;
	virtual void Music_Start() override;
	virtual void Music_End() override;

	virtual UAudioComponent* GetAudioComponent() override;
	
	/**
	 * Get Last Active Audio Component
	 * @return Last Active Audio Component
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	virtual UAudioComponent* GetLastActiveAudioComponent() const;

	/**
	 * Get Current Active Audio Component
	 * @return Current Active Audio Component
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	virtual UAudioComponent* GetActiveAudioComponent() const;
public:
	// Fade Audio Setting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FDreamMusicPlayerFadeAudioSetting FadeAudioSetting;
	
	// If ture SubB else SubA
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool CurrentActiveAudioComponent = false;

	// A Audio Component
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentA = nullptr;

	// B Audio Component
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentB = nullptr;


protected:
	/**
	 * Toggle Active Audio Component
	 * @return Whether the A audio component is active
	 */
	bool ToggleActiveAudioComponent();

private:
	FTimerHandle StopTimerHandle;
};
