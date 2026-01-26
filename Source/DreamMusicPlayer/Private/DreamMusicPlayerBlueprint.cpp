// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerBlueprint.h"

#include "Classes/DreamMusicPlayerExpansionData.h"
#include "Classes/DreamMusicPlayerComponent.h"

#include "Classes/DreamMusicDataAsset.h"
#include "DreamMusicTimestamp.h"
#include "Interface/DreamMusicPlayerInterfaces.h"
#include "DreamMusicData.h"


float UDreamMusicPlayerBlueprint::ConvLyricTimestampToFloat(FDreamMusicTimestamp InTimestamp)
{
	return InTimestamp.ToSeconds();
}

FDreamMusicTimestamp UDreamMusicPlayerBlueprint::ConvFloatToLyricTimestamp(float InFloat)
{
	return *FDreamMusicTimestamp().FromSeconds(InFloat);
}

bool UDreamMusicPlayerBlueprint::GetExpansionDataByClass(const FDreamMusicData& InMusicData, TSubclassOf<UDreamMusicPlayerExpansionData> InExpansionDataClass, UDreamMusicPlayerExpansionData*& OutExpansionData)
{
	for (UDreamMusicPlayerExpansionData* ExpansionData : InMusicData.ExpansionData)
	{
		if (!IsValid(ExpansionData))
		{
			continue;
		}

		if (ExpansionData->GetClass()->IsChildOf(InExpansionDataClass))
		{
			OutExpansionData = ExpansionData;
			return true;
		}
	}

	return false;
}

TArray<FDreamMusicData> UDreamMusicPlayerBlueprint::GetArtistMusics(UDataTable* InArtistDataTable, FName InArtistName)
{
	TArray<FDreamMusicData> Cache;

	for (const FName& RowName : InArtistDataTable->GetRowNames())
	{
		FDreamMusicPlayerSongList* Data = InArtistDataTable->FindRow<FDreamMusicPlayerSongList>(RowName, FString(), true);
		if (Data && IsValid(Data->MusicData))
		{
			if (Data->MusicData->Data.Tag.Artist == InArtistName)
			{
				Cache.Add(Data->MusicData->Data);
			}
		}
	}

	return Cache;
}

TArray<FDreamMusicData> UDreamMusicPlayerBlueprint::GetAlbumMusics(UDataTable* InAlbumDataTable, FName InAlbumName)
{
	TArray<FDreamMusicData> Cache;

	for (const FName& RowName : InAlbumDataTable->GetRowNames())
	{
		FDreamMusicPlayerSongList* Data = InAlbumDataTable->FindRow<FDreamMusicPlayerSongList>(RowName, FString(), true);
		if (Data && IsValid(Data->MusicData))
		{
			if (Data->MusicData->Data.Tag.Album == InAlbumName)
			{
				Cache.Add(Data->MusicData->Data);
			}
		}
	}

	return Cache;
}

TArray<FDreamMusicData> UDreamMusicPlayerBlueprint::FilterMusicByTitle(TArray<FDreamMusicData> InMusicDatas, FString InTitle)
{
	return InMusicDatas.FilterByPredicate([InTitle](const FDreamMusicData& InData)
	{
		return InData.Tag.Title == InTitle;
	});
}

UDreamMusicPlayerComponent* UDreamMusicPlayerBlueprint::GetDreamMusicPlayerComponent(AActor* InActor)
{
	if (UDreamMusicPlayerComponent* Comp = GetDreamMusicPlayerComponentByInterface(InActor))
	{
		return Comp;
	}
	return InActor->FindComponentByClass<UDreamMusicPlayerComponent>();
}

UDreamMusicPlayerComponent* UDreamMusicPlayerBlueprint::GetDreamMusicPlayerComponentByInterface(AActor* InActor)
{
	if (const IDreamMusicPlayerCommonInterface* Interface = Cast<IDreamMusicPlayerCommonInterface>(InActor))
	{
		return Interface->Execute_GetDreamMusicPlayerComponent(InActor);
	}
	return nullptr;
}
