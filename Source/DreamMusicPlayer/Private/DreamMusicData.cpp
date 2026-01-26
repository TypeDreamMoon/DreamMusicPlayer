#include "DreamMusicData.h"
#include "Classes/DreamMusicPlayerExpansionData.h"

bool FDreamMusicData::IsValid() const
{
	return Tag.IsValid() && Music.IsValid();
}

bool FDreamMusicData::operator==(const FDreamMusicData& Target) const
{
	return Tag == Target.Tag && Music == Target.Music;
}

bool FDreamMusicData::HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const
{
	for (UDreamMusicPlayerExpansionData* ExpansionData : ExpansionData)
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
