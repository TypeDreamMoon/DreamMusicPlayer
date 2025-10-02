// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerBlueprint.generated.h"

class UDreamMusicPlayerExpansionData;
struct FDreamMusicDataStruct;
struct FDreamMusicLyricTimestamp;
/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYER_API UDreamMusicPlayerBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Lyric")
	static TArray<FString> GetLyricFileNames();

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Types")
	static float ConvLyricTimestampToFloat(FDreamMusicLyricTimestamp InTimestamp);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Types")
	static FDreamMusicLyricTimestamp ConvFloatToLyricTimestamp(float InFloat);

	UFUNCTION(BlueprintPure, Category = "DreamMusicPlayer|Functions|Expansion", Meta = (DeterminesOutputType="InExpansionDataClass", DynamicOutputParam="OutExpansionData"))
	static bool GetExpansionDataByClass(const FDreamMusicDataStruct& InMusicData, TSubclassOf<UDreamMusicPlayerExpansionData> InExpansionDataClass, UDreamMusicPlayerExpansionData*& OutExpansionData);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicDataStruct> GetArtistMusics(UDataTable* InArtistDataTable, FName InArtistName);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicDataStruct> GetAlbumMusics(UDataTable* InAlbumDataTable, FName InAlbumName);

	UFUNCTION(BlueprintCallable, Category = "DreamMusicPlayer|Functions|MusicInformation")
	static TArray<FDreamMusicDataStruct> FilterMusicByTitle(TArray<FDreamMusicDataStruct> InMusicDatas, FString InTitle);
};
