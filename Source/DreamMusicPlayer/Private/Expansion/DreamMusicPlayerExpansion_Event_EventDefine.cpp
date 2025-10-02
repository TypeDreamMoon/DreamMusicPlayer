// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_Event_EventDefine.h"

#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerLog.h"


void UDreamMusicPlayerExpansion_Event_EventDefine::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
}

UObject* UDreamMusicPlayerExpansion_Event_EventDefine::GetPayload() const
{
	return Payload;
}

void UDreamMusicPlayerExpansion_Event_EventDefine::CallEvent(const FString& EventName, const FDreamMusicLyric& Lyric, UDreamMusicPlayerPayload* InEventPayload)
{
	if (EventName.IsEmpty())
	{
		return;
	}

	UFunction* Function = FindFunction(FName(EventName));

	DMP_LOG(Log, TEXT("Call Event: %s"), *EventName);

	struct FParams
	{
		FParams(const FDreamMusicLyric& Lyric, UDreamMusicPlayerPayload* InPayload)
			: Lyric(Lyric), EventPayload(InPayload)
		{
		}

		FDreamMusicLyric Lyric;
		UDreamMusicPlayerPayload* EventPayload;
	};

	FParams Params = FParams(Lyric, InEventPayload);

	ProcessEvent(Function, &Params);
}

void UDreamMusicPlayerExpansion_Event_EventDefine::CallEvent(FDreamEventDefine Event)
{
	CallEvent(Event.Key, FDreamMusicLyric(), Event.Value);
}

class UWorld* UDreamMusicPlayerExpansion_Event_EventDefine::GetWorld() const
{
	return GWorld;
}
