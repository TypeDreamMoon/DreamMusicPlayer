// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerBlueprint.h"

#include "LyricParser/DreamMusicPlayerLyricTools.h"

#include "Classes/DreamMusicPlayerExpansionData.h"
#include "Classes/DreamMusicData.h"

TArray<FString> UDreamMusicPlayerBlueprint::GetLyricFileNames()
{
	return FDreamMusicPlayerLyricTools::GetLyricFileNames();
}

float UDreamMusicPlayerBlueprint::ConvLyricTimestampToFloat(FDreamMusicLyricTimestamp InTimestamp)
{
	return InTimestamp.ToSeconds();
}

FDreamMusicLyricTimestamp UDreamMusicPlayerBlueprint::ConvFloatToLyricTimestamp(float InFloat)
{
	return *FDreamMusicLyricTimestamp().FromSeconds(InFloat);
}

bool UDreamMusicPlayerBlueprint::GetExpansionDataByClass(const FDreamMusicDataStruct& InMusicData, TSubclassOf<UDreamMusicPlayerExpansionData> InExpansionDataClass, UDreamMusicPlayerExpansionData*& OutExpansionData)
{
	for (UDreamMusicPlayerExpansionData* ExpansionData : InMusicData.ExpansionDatas)
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

TArray<FDreamMusicDataStruct> UDreamMusicPlayerBlueprint::GetArtistMusics(UDataTable* InArtistDataTable, FName InArtistName)
{
	TArray<FDreamMusicDataStruct> Cache;

	for (const FName& RowName : InArtistDataTable->GetRowNames())
	{
		FDreamMusicPlayerSongList* Data = InArtistDataTable->FindRow<FDreamMusicPlayerSongList>(RowName, FString(), true);
		if (Data && IsValid(Data->MusicData))
		{
			if (Data->MusicData->Data.Information.Artist == InArtistName)
			{
				Cache.Add(Data->MusicData->Data);
			}
		}
	}

	return Cache;
}

TArray<FDreamMusicDataStruct> UDreamMusicPlayerBlueprint::GetAlbumMusics(UDataTable* InAlbumDataTable, FName InAlbumName)
{
	TArray<FDreamMusicDataStruct> Cache;

	for (const FName& RowName : InAlbumDataTable->GetRowNames())
	{
		FDreamMusicPlayerSongList* Data = InAlbumDataTable->FindRow<FDreamMusicPlayerSongList>(RowName, FString(), true);
		if (Data && IsValid(Data->MusicData))
		{
			if (Data->MusicData->Data.Information.Album == InAlbumName)
			{
				Cache.Add(Data->MusicData->Data);
			}
		}
	}

	return Cache;
}

TArray<FDreamMusicDataStruct> UDreamMusicPlayerBlueprint::FilterMusicByTitle(TArray<FDreamMusicDataStruct> InMusicDatas, FString InTitle)
{
	return InMusicDatas.FilterByPredicate([InTitle](const FDreamMusicDataStruct& InData)
	{
		return InData.Information.Title == InTitle;
	});
}