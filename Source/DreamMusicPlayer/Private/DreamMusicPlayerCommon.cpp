#include "DreamMusicPlayerCommon.h"


#include "Classes/DreamMusicPlayerExpansionData.h"


bool FDreamMusicInformation::IsValid() const
{
	return !Title.IsEmpty() || !Artist.IsEmpty() || !Album.IsEmpty() || Cover.IsValid() || !Genre.IsEmpty();
}

bool FDreamMusicInformation::operator==(const FDreamMusicInformation& Target) const
{
	return Title == Target.Title && Artist == Target.Artist && Album == Target.Album && Genre == Target.Genre && Cover == Target.Cover;
}

bool FDreamMusicInformationData::IsValid() const
{
	return Music.LoadSynchronous() != nullptr;
}

bool FDreamMusicInformationData::operator==(const FDreamMusicInformationData& Target) const
{
	return Music == Target.Music;
}

bool FDreamMusicDataStruct::IsValid() const
{
	return Information.IsValid() && Data.IsValid();
}

bool FDreamMusicDataStruct::operator==(const FDreamMusicDataStruct& Target) const
{
	return Information == Target.Information && Data == Target.Data;
}

bool FDreamMusicDataStruct::HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const
{
	for (UDreamMusicPlayerExpansionData* ExpansionData : ExpansionDatas)
	{
		if (ExpansionData == nullptr)
		{
			continue;
		}

		if (ExpansionData->GetClass() == ExpansionDataClass)
		{
			return true;
		}
	}

	return false;
}
