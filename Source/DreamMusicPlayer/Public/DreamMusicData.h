#pragma once

#include "CoreMinimal.h"
#include "DreamMusicTag.h"
#include "DreamMusicData.generated.h"

class UDreamMusicPlayerExpansionData;

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
	TSoftObjectPtr<USoundBase> Music;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UDreamMusicPlayerExpansionData*> ExpansionData;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicData& Target) const;

	bool HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const;

	template <typename T>
	T* GetExpansionData() const
	{
		for (auto ExpansionData : ExpansionData)
		{
			if (auto CastedExpansionData = Cast<T>(ExpansionData))
			{
				return CastedExpansionData;
			}
		}
		return nullptr;
	}
};
