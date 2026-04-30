#pragma once

#include "CoreMinimal.h"
#include "DreamMusicTag.h"
#include "DreamMusicData.generated.h"

class UDreamMusicPlayerExpansionData;

UENUM(BlueprintType)
enum class EDreamMusicPlayerMusicType : uint8
{
	Asset = 0,
	Network = 1,
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicData
{
	GENERATED_BODY()

public:
	FDreamMusicData() : Tag(FDreamMusicTag()), Music(nullptr)
	{
	};

	FDreamMusicData(FDreamMusicTag InTag, TSoftObjectPtr<USoundBase> InMusic)
		: Tag(InTag), Music(InMusic)
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDreamMusicPlayerMusicType MusicType = EDreamMusicPlayerMusicType::Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "MusicType == EDreamMusicPlayerMusicType::Network"))
	FString MusicURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "MusicType == EDreamMusicPlayerMusicType::Asset"))
	TSoftObjectPtr<USoundBase> Music;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UDreamMusicPlayerExpansionData*> ExpansionData;
	
	UPROPERTY(Transient)
	TObjectPtr<USoundWave> CachedMusic;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicData& Target) const;

	bool HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const;

	template <typename T>
	T* GetExpansionData() const
	{
		for (auto Element : ExpansionData)
		{
			if (auto CastedExpansionData = Cast<T>(Element))
			{
				return CastedExpansionData;
			}
		}
		return nullptr;
	}
};
