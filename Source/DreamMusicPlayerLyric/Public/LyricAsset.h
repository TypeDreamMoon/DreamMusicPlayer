#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LyricAsset.generated.h"

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricTimeSpan
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Hours = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Minutes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Seconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Milliseconds = 0;

	FLyricTimeSpan() = default;
	FLyricTimeSpan(int32 H, int32 M, int32 S, int32 MS) : Hours(H), Minutes(M), Seconds(S), Milliseconds(MS) {}

	int64 ToTotalMilliseconds() const;
};

UENUM(BlueprintType)
enum class ELyricTextRole : uint8
{
	None = 0,
	Lyric = 1,
	Romanization = 2,
	Translation = 4
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricWord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLyricTimeSpan StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLyricTimeSpan EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasEndTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELyricTextRole Role = ELyricTextRole::Lyric;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELyricTextRole Role = ELyricTextRole::Lyric;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLyricWord> Words;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLyricTimeSpan Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLyricLine> Lines;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Items;
};

UCLASS(BlueprintType)
class DREAMMUSICPLAYERLYRIC_API ULyricAsset : public UObject
{
	GENERATED_BODY()

public:
	ULyricAsset(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric")
	FLyricMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric")
	TArray<FLyricGroup> Groups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric")
	FString SourceFileName;
};

