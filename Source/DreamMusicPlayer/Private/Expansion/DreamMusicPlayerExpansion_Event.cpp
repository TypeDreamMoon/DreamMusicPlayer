// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_Event.h"

#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "Expansion/DreamMusicPlayerExpansion_Lyric.h"
#include "Expansion/DreamMusicPlayerExpansion_Event_EventDefine.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Event.h"


void UDreamMusicPlayerExpansion_Event::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
	EventDefineObject->SetPayload(InPayloadObject);
}

void UDreamMusicPlayerExpansion_Event::Initialize(UDreamMusicPlayerComponent* InComponent)
{
	Super::Initialize(InComponent);

	if (InComponent->HasExpansion(UDreamMusicPlayerExpansion_Lyric::StaticClass()))
	{
		InComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>()->OnLyricChangedNative.AddUObject(
			this, &UDreamMusicPlayerExpansion_Event::OnLyricChangedHandle);
	}
	else
	{
		DMP_LOG(Error, TEXT("Lyric Expansion Not Found, request Lyric Expansion"));
	}
}

void UDreamMusicPlayerExpansion_Event::BP_MusicStart_Implementation()
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_Event::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_Event_EventDefine& Define : CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_Event>()->MusicStartEventDefines)
		{
			Define.Event.Call([this](FDreamEventDefine Event)
			{
				EventDefineObject->CallEvent(Event);
			});
		}
	}
}

void UDreamMusicPlayerExpansion_Event::BP_MusicEnd_Implementation()
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_Event::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_Event_EventDefine& Define : CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_Event>()->MusicEndEventDefines)
		{
			Define.Event.Call([this](FDreamEventDefine Event)
			{
				EventDefineObject->CallEvent(Event);
			});
		}
	}
}

void UDreamMusicPlayerExpansion_Event::BP_MusicSetPercent_Implementation(float InPercent)
{
	IgnoreTimestamp.Empty();
}

void UDreamMusicPlayerExpansion_Event::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_Event::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_Event_TimeEventDefine& Define : CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_Event>()->TimeEventDefines)
		{
			if (!IgnoreTimestamp.Contains(Define.Time) && Define.Time.IsApproximatelyEqual(InTimestamp, TimeEventToleranceMilliseconds))
			{
				Define.Event.Call([this](FDreamEventDefine Event)
				{
					EventDefineObject->CallEvent(Event.Key, MusicPlayerComponent->GetExpansion<UDreamMusicPlayerExpansion_Lyric>()->CurrentLyric, Event.Value);
				});

				IgnoreTimestamp.Add(Define.Time);
				return;
			}
		}
	}
}

void UDreamMusicPlayerExpansion_Event::OnLyricChangedHandle(FDreamMusicLyric Lyric, int Index)
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_Event::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_Event_EventDefine& Define : CurrentMusicData.GetExpansionData<
			     UDreamMusicPlayerExpansionData_Event>()->LyricEventDefines)
		{
			if (Define == Index)
			{
				Define.Event.Call([this, Lyric](FDreamEventDefine Event)
				{
					EventDefineObject->CallEvent(Event.Key, Lyric, Event.Value);
				});

				return;
			}
		}
	}
}
