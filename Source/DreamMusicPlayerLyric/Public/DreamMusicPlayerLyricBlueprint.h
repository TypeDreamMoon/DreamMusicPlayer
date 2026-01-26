// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamLyricTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DreamMusicPlayerLyricBlueprint.generated.h"

USTRUCT(BlueprintType)
struct FDreamMusicLyricSearchResult
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricGroup Group;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double SimilarityScore = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MatchedText;
};

/**
 * 
 */
UCLASS()
class DREAMMUSICPLAYERLYRIC_API UDreamMusicPlayerLyricBlueprint : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Dream Music Player|Lyric")
	static TArray<FString> GetLyricFileNames();

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	static FDreamMusicLyricGroup FindGroupAtTime(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	static int32 FindGroupIndexAtTime(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	static FDreamMusicLyricGroup FindCurrentOrPrevGroup(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	static FDreamMusicLyricProgress GetLineWordProgress(const FDreamMusicLyricLine& Line, const FDreamMusicTimestamp& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	static FDreamMusicLyricProgress GetLineProgress(const FDreamMusicLyricLine& Line, const FDreamMusicTimestamp& Timestamp);

	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric")
	TArray<FDreamMusicLyricSearchResult> SearchLyric(const TArray<FDreamMusicLyricGroup>& Groups, const FString& Query, double Threshold = 0.5);
};
