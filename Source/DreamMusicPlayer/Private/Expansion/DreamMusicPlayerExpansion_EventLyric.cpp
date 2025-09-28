// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_EventLyric.h"

#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "Expansion/DreamMusicPlayerExpansion_EventLyric_EventDefine.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_EventLyric.h"
#include "Expansion/DreamMusicPlayerExpansion_Lyric.h"

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

void UDreamMusicPlayerExpansion_EventLyric::OnLyricChangedHandle(FDreamMusicLyric Lyric, int Index)
{
	if (CurrentMusicData.HasExpansionData(UDreamMusicPlayerExpansionData_EventLyric::StaticClass()))
	{
		for (const FDreamMusicPlayerExpansionData_EventLyric_EventDefine& Define : CurrentMusicData.GetExpansionData<
			     UDreamMusicPlayerExpansionData_EventLyric>()->EventDefines)
		{
			if (Define == Index)
			{
				EventDefineObject->CallEvent(Define.EventName, Lyric);

				return;
			}
		}
	}
	else
	{
		DMP_LOG(Error, TEXT("Lyric ExpansionData Not Found, request Lyric ExpansionData"));
	}
}
