// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "ExpansionData/DreamMusicPlayerExpansionData_Event.h"
#include "DreamMusicPlayerExpansion_Event.generated.h"

class UDreamMusicPlayerExpansion_Event_EventDefine;

/**
 * Event Lyric扩展类，用于处理音乐播放过程中的歌词事件
 * 继承自UDreamMusicPlayerExpansion基类，提供歌词事件定义和负载数据管理功能
 */
UCLASS(DisplayName = "Event")
class DREAMMUSICPLAYER_API UDreamMusicPlayerExpansion_Event : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int TimeEventToleranceMilliseconds = 5;

	/**
	 * 事件定义对象，用于配置歌词事件的具体行为
	 * 该对象在编辑器中可配置，并且是实例化的对象
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Event")
	UDreamMusicPlayerExpansion_Event_EventDefine* EventDefineObject;

public:
	/**
	 * 设置负载对象，用于传递事件相关的数据
	 * @param InPayloadObject 要设置的负载对象指针
	 */
	UFUNCTION(BlueprintCallable)
	void SetPayload(UObject* InPayloadObject);

protected:
	/**
	 * 存储负载对象的弱引用指针，避免循环引用问题
	 */
	TWeakObjectPtr<UObject> Payload;

protected:
	virtual void Initialize(UDreamMusicPlayerComponent* InComponent) override;
	virtual void BP_MusicStart_Implementation() override;
	virtual void BP_MusicEnd_Implementation() override;
	virtual void BP_MusicSetPercent_Implementation(float InPercent) override;
	virtual void BP_Tick_Implementation(const FDreamMusicLyricTimestamp& InTimestamp, float InDeltaTime) override;

	/**
	 * 歌词变更事件处理函数
	 * 当歌词发生变化时被调用，处理当前歌词和索引相关的逻辑
	 * @param Lyric 当前歌词信息结构体
	 * @param Index 当前歌词在列表中的索引位置
	 */
	void OnLyricChangedHandle(FDreamMusicLyric Lyric, int Index);

	TArray<FDreamMusicLyricTimestamp> IgnoreTimestamp;
};
