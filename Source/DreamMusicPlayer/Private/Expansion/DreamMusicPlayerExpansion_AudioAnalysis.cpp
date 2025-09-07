// Fill out your copyright notice in the Description page of Project Settings.


#include "Expansion/DreamMusicPlayerExpansion_AudioAnalysis.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "ConstantQNRT.h"
#include "LoudnessNRT.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_AudioAnalysis.h"

void UDreamMusicPlayerExpansion_AudioAnalysis::GetAudioNrtData(bool bConstantReverse, TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR,
                                                               TArray<float>& ConstantNrtAverage,
                                                               float& OutLoudnessValue)
{
	ConstantNrtL = ConstantQDataL;
	ConstantNrtR = ConstantQDataR;
	TArray<float> ConstantQData, ConstantQDataAverage;
	if (bConstantReverse)
	{
		ConstantQData.Append(ConstantQDataR);
		ConstantQData.Append(ConstantQDataL);
	}
	else
	{
		ConstantQData.Append(ConstantQDataL);
		ConstantQData.Append(ConstantQDataR);
	}

	for (int i = 0; i < ConstantQData.Num() / 2; ++i)
	{
		ConstantQDataAverage.Add((ConstantQData[i] + ConstantQData[ConstantQData.Num() - i - 1]) / 2);
	}
	ConstantNrtAverage = ConstantQDataAverage;
	OutLoudnessValue = LoudnessValue;
}

void UDreamMusicPlayerExpansion_AudioAnalysis::UpdateAudioAnalysisData()
{
	if (ConstantQ && ConstantQ->IsValidLowLevel())
	{
		ConstantQ->GetNormalizedChannelConstantQAtTime(MusicPlayerComponent->CurrentDuration, 0, ConstantQDataL);
		ConstantQ->GetNormalizedChannelConstantQAtTime(MusicPlayerComponent->CurrentDuration, 1, ConstantQDataR);
	}

	if (Loudness && Loudness->IsValidLowLevel())
	{
		Loudness->GetNormalizedLoudnessAtTime(MusicPlayerComponent->CurrentDuration, LoudnessValue);
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_ChangeMusic_Implementation(const FDreamMusicDataStruct& InData)
{
	LoadAudioNrt();
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime)
{
	UpdateAudioAnalysisData();
}

void UDreamMusicPlayerExpansion_AudioAnalysis::LoadAudioNrt()
{
	FSoftObjectPath CQ = CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_AudioAnalysis>()->ConstantQ;
	FSoftObjectPath LN = CurrentMusicData.GetExpansionData<UDreamMusicPlayerExpansionData_AudioAnalysis>()->Loudness;
	if (CQ.IsValid())
		ConstantQ = Cast<UConstantQNRT>(CQ.TryLoad());
	if (LN.IsValid())
		Loudness = Cast<ULoudnessNRT>(LN.TryLoad());
}
