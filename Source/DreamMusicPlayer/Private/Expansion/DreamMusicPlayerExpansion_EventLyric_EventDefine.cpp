// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_EventLyric_EventDefine.h"

#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerLog.h"


void UDreamMusicPlayerExpansion_EventLyric_EventDefine::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
}

UObject* UDreamMusicPlayerExpansion_EventLyric_EventDefine::GetPayload() const
{
	return Payload;
}

void UDreamMusicPlayerExpansion_EventLyric_EventDefine::CallEvent(const FString& EventName, const FDreamMusicLyric& Lyric, UDreamMusicPlayerPayload* InEventPayload)
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

class UWorld* UDreamMusicPlayerExpansion_EventLyric_EventDefine::GetWorld() const
{
	return GWorld;
}
