// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_EventLyric_EventDefine.h"

#include "DreamMusicPlayerCommon.h"


void UDreamMusicPlayerExpansion_EventLyric_EventDefine::SetPayload(UObject* InPayloadObject)
{
	Payload = InPayloadObject;
}

UObject* UDreamMusicPlayerExpansion_EventLyric_EventDefine::GetPayload() const
{
	return Payload;
}

void UDreamMusicPlayerExpansion_EventLyric_EventDefine::CallEvent(const FString& EventName,
                                                                  const FDreamMusicLyric& Lyric)
{
	if (EventName.IsEmpty())
	{
		return;
	}
	
	UFunction* Function = FindFunction(FName(EventName));
	
	struct FParams
	{
		FParams(const FDreamMusicLyric& Lyric)
			: Lyric(Lyric)
		{
		}

		FDreamMusicLyric Lyric;
	};
	
	FParams Params = FParams(Lyric);

	ProcessEvent(Function, &Params);
}

class UWorld* UDreamMusicPlayerExpansion_EventLyric_EventDefine::GetWorld() const
{
	return GWorld;
}
