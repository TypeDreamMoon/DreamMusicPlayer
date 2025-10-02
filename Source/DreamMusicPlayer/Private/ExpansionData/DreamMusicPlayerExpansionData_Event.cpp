// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpansionData/DreamMusicPlayerExpansionData_Event.h"

void FDreamMusicPlayerExpansionData_BaseEvent::Call(FDreamEventCallback Callback) const
{
	for (const TPair<FString, UDreamMusicPlayerPayload*>& Event : Events)
	{
		Callback(Event);
	}
}

bool FDreamMusicPlayerExpansionData_Event_EventDefine::operator==(int Other) const
{
	return Other == Index;
}

bool FDreamMusicPlayerExpansionData_Event_TimeEventDefine::operator==(const FDreamMusicLyricTimestamp& Other) const
{
	return Other == Time;
}
