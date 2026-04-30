#include "DreamMusicData.h"
#include "Classes/DreamMusicPlayerExpansionData.h"

bool FDreamMusicData::IsValid() const
{
	return Tag.IsValid() && MusicType == EDreamMusicPlayerMusicType::Network ? true : Music.IsValid();
}

bool FDreamMusicData::operator==(const FDreamMusicData& Target) const
{
	return Tag == Target.Tag && Music == Target.Music;
}

bool FDreamMusicData::HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const
{
	for (UDreamMusicPlayerExpansionData* Expansion : ExpansionData)
	{
		if (Expansion == nullptr)
		{
			continue;
		}

		if (Expansion->GetClass() == ExpansionDataClass)
		{
			return true;
		}
	}

	return false;
}
