#include "Classes/DreamMusicPlayerComponent.h"


FDreamMusicData UDreamMusicPlayerComponent::GetNextMusicData(FDreamMusicData InData)
{
	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Loop)
	{
		return CurrentMusicData.IsValid() ? CurrentMusicData : InData;
	}
	if (MusicDataList.Contains(InData))
	{
		return MusicDataList[(MusicDataList.Find(InData) + 1) > MusicDataList.Num() - 1
			                     ? 0
			                     : MusicDataList.Find(InData) + 1];
	}
	else
	{
		return MusicDataList[0];
	}
}

FDreamMusicData UDreamMusicPlayerComponent::GetLastMusicData(FDreamMusicData InData)
{
	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Loop)
	{
		return CurrentMusicData.IsValid() ? CurrentMusicData : InData;
	}
	return MusicDataList[(MusicDataList.Find(InData) - 1 < 0)
		                     ? MusicDataList.Num() - 1
		                     : MusicDataList.Find(InData) - 1];
}

void UDreamMusicPlayerComponent::GetExpansionByClass(TSubclassOf<UDreamMusicPlayerExpansion> InExpansionClass, UDreamMusicPlayerExpansion*& OutExpansion) const
{
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		if (Expansion->GetClass() == InExpansionClass)
		{
			OutExpansion = Expansion;
			return;
		}
	}
}

bool UDreamMusicPlayerComponent::HasExpansion(TSubclassOf<UDreamMusicPlayerExpansion> InExpansionClass) const
{
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		if (Expansion->GetClass() == InExpansionClass)
		{
			return true;
		}
	}

	return false;
}
