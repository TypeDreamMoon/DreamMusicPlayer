// Fill out your copyright notice in the Description page of Project Settings.


#include "ExpansionData/DreamMusicPlayerExpansionData_Event.h"

#include "DreamLyricTypes.h"

void FDreamMusicPlayerExpansionData_BaseEvent::Call(FDreamEventCallback Callback) const
{
	for (const FDreamMusicPlayerExpansionData_BaseEvent_SingleEventDefine& Event : Events)
	{
		Callback(Event);
	}
}

bool FDreamMusicPlayerExpansionData_Event_LyricEventDefine::operator==(int Other) const
{
	return Other == Index;
}

bool FDreamMusicPlayerExpansionData_Event_TimeEventDefine::operator==(const FDreamMusicTimestamp& Other) const
{
	return Other == Time;
}
