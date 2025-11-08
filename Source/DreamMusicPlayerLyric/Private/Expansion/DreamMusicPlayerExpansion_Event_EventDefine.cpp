// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_Event_EventDefine.h"

#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerDebugLog.h"
#include "DreamMusicPlayerLog.h"


void UDreamMusicPlayerExpansion_Event_EventDefine::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
}

UObject* UDreamMusicPlayerExpansion_Event_EventDefine::GetPayload() const
{
	return Payload;
}

void UDreamMusicPlayerExpansion_Event_EventDefine::CallEvent(const FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine& InEvent, const FDreamMusicLyric& InLyric)
{
	if (InEvent.EventName.IsNone() || InEvent.Payload == nullptr)
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Invalid Event"));
		return;
	}

	UFunction* Function = FindFunction(InEvent.EventName);
	if (Function == nullptr)
	{
		DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Invalid Function"));
		return;
	}

	struct FParams
	{
		FParams(const FDreamMusicLyric& Lyric, UDreamMusicPlayerPayload* InPayload)
			: Lyric(Lyric), EventPayload(InPayload)
		{
		}

		FParams(const FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine& InEvent, const FDreamMusicLyric& InLyric)
			: Lyric(InLyric), EventPayload(InEvent.Payload)
		{
		}

		FDreamMusicLyric Lyric;
		UDreamMusicPlayerPayload* EventPayload;
	};

	FParams Params(InEvent, InLyric);
	
	ProcessEvent(Function, &Params);
}

void UDreamMusicPlayerExpansion_Event_EventDefine::CallEvent(const FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine& InEvent)
{
	CallEvent(InEvent, FDreamMusicLyric());
}

class UWorld* UDreamMusicPlayerExpansion_Event_EventDefine::GetWorld() const
{
	return GWorld;
}
