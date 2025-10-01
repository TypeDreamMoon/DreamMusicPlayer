// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_EventLyric.h"

#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "Expansion/DreamMusicPlayerExpansion_Lyric.h"
#include "Expansion/DreamMusicPlayerExpansion_EventLyric_EventDefine.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_EventLyric.h"


void UDreamMusicPlayerExpansion_EventLyric::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
	EventDefineObject->SetPayload(InPayloadObject);
}

void UDreamMusicPlayerExpansion_EventLyric::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);

	if (InComponent->HasExpansion(UDreamMusicPlayerExpansion_Lyric::StaticClass()))
	{
		InComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>()->OnLyricChangedNative.AddUObject(
			this, &UDreamMusicPlayerExpansion_EventLyric::OnLyricChangedHandle);
	}
	else
	{
		DMP_LOG(Error, TEXT("Lyric Expansion Not Found, request Lyric Expansion"));
	}
}

void UDreamMusicPlayerExpansion_EventLyric::BP_MusicSetPercent_Implementation(float InPercent)
{
	IgnoreTimestamp.Empty();
}

void UDreamMusicPlayerExpansion_EventLyric::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_EventLyric::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_EventLyric_TimeEventDefine& Define : CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_EventLyric>()->TimeEventDefines)
		{
			if (!IgnoreTimestamp.Contains(Define.Time) && Define.Time.IsApproximatelyEqual(InTimestamp, TimeEventToleranceMilliseconds))
			{
				for (const TPair<FString, UDreamMusicPlayerPayload*>& Event : Define.Events)
				{
					EventDefineObject->CallEvent(Event.Key, MusicPlayerComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>()->CurrentLyric, Event.Value);					
				}
				
				IgnoreTimestamp.Add(Define.Time);
				return;
			}
		}
	}
}

void UDreamMusicPlayerExpansion_EventLyric::OnLyricChangedHandle(FDreamMusicLyric Lyric, int Index)
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_EventLyric::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_EventLyric_EventDefine& Define : CurrentMusicData.GetExpansionData<
			     UDreamMusicPlayerExpansionData_EventLyric>()->EventDefines)
		{
			if (Define == Index)
			{
				for (const TPair<FString, UDreamMusicPlayerPayload*>& Event : Define.Events)
				{
					EventDefineObject->CallEvent(Event.Key, MusicPlayerComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>()->CurrentLyric, Event.Value);					
				}

				return;
			}
		}
	}
}
