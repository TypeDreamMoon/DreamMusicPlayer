// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerBlueprint.generated.h"

class UDreamMusicPlayerComponent;
class UDreamMusicPlayerExpansionData;
struct FDreamMusicData;
struct FDreamMusicTimestamp;
/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicPlayerBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Types")
	static float ConvLyricTimestampToFloat(FDreamMusicTimestamp InTimestamp);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Types")
	static FDreamMusicTimestamp ConvFloatToLyricTimestamp(float InFloat);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Expansion", Meta = (DeterminesOutputType="InExpansionDataClass", DynamicOutputParam="OutExpansionData"))
	static bool GetExpansionDataByClass(const FDreamMusicData& InMusicData, TSubclassOf<UDreamMusicPlayerExpansionData> InExpansionDataClass, UDreamMusicPlayerExpansionData*& OutExpansionData);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicData> GetArtistMusics(UDataTable* InArtistDataTable, FName InArtistName);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicData> GetAlbumMusics(UDataTable* InAlbumDataTable, FName InAlbumName);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicData> FilterMusicByTitle(TArray<FDreamMusicData> InMusicDatas, FString InTitle);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions")
	static UDreamMusicPlayerComponent* GetDreamMusicPlayerComponent(AActor* InActor);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions")
	static UDreamMusicPlayerComponent* GetDreamMusicPlayerComponentByInterface(AActor* InActor);
};
