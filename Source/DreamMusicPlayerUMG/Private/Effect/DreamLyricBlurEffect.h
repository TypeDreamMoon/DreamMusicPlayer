// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDreamLyricEffect.h"

/**
 * 模糊效果模式
 */
UENUM(BlueprintType)
enum class EDreamLyricBlurMode : uint8
{
	/** 未播放文字模糊 */
	Unplayed,
	
	/** 已播放文字模糊 */
	Played,
	
	/** 非当前播放文字模糊（突出当前） */
	NonCurrent,
	
	/** 渐进式模糊（距离越远越模糊） */
	Distance
};

/**
 * 模糊效果
 * 注意：实际模糊需要通过多次渲染偏移文本来模拟
 */
class DREAMMUSICPLAYERUMG_API FDreamLyricBlurEffect : public IDreamLyricEffect
{
public:
	FDreamLyricBlurEffect();

	virtual void ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const override;
	virtual bool IsEnabled() const override { return bEnabled; }
	virtual void SetEnabled(bool bInEnabled) override { bEnabled = bInEnabled; }
	virtual int32 GetPriority() const override { return 40; } // 模糊效果优先级较低

	// 配置
	void SetBlurMode(EDreamLyricBlurMode InMode) { BlurMode = InMode; }
	void SetMaxBlurRadius(float InRadius) { MaxBlurRadius = FMath::Max(0.0f, InRadius); }
	void SetBlurRange(int32 InRange) { BlurRange = FMath::Max(1, InRange); }
	void SetBlurOpacity(float InOpacity) { BlurOpacity = FMath::Clamp(InOpacity, 0.0f, 1.0f); }

	EDreamLyricBlurMode GetBlurMode() const { return BlurMode; }
	float GetMaxBlurRadius() const { return MaxBlurRadius; }

private:
	float CalculateBlurAmount(float Distance) const;

	bool bEnabled;
	EDreamLyricBlurMode BlurMode;
	float MaxBlurRadius;   // 最大模糊半径（用于模拟模糊的偏移量）
	int32 BlurRange;       // 模糊影响范围
	float BlurOpacity;     // 模糊层透明度
};

// Implementation
inline FDreamLyricBlurEffect::FDreamLyricBlurEffect()
	: bEnabled(false)
	, BlurMode(EDreamLyricBlurMode::Unplayed)
	, MaxBlurRadius(2.0f)
	, BlurRange(5)
	, BlurOpacity(0.3f)
{
}

inline void FDreamLyricBlurEffect::ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const
{
	if (!bEnabled || Context.TotalUnits == 0)
	{
		return;
	}

	float TargetIndex = Context.CurrentProgress * Context.TotalUnits;
	float Distance = Context.Unit.GlobalIndex - TargetIndex;
	float BlurAmount = 0.0f;

	switch (BlurMode)
	{
	case EDreamLyricBlurMode::Unplayed:
		// 未播放的文字模糊
		if (Distance > 0.5f)
		{
			BlurAmount = CalculateBlurAmount(Distance);
		}
		break;

	case EDreamLyricBlurMode::Played:
		// 已播放的文字模糊
		if (Distance < -0.5f)
		{
			BlurAmount = CalculateBlurAmount(FMath::Abs(Distance));
		}
		break;

	case EDreamLyricBlurMode::NonCurrent:
		// 非当前播放的文字模糊
		if (FMath::Abs(Distance) > 0.5f)
		{
			BlurAmount = CalculateBlurAmount(FMath::Abs(Distance));
		}
		break;

	case EDreamLyricBlurMode::Distance:
		// 距离越远越模糊
		BlurAmount = CalculateBlurAmount(FMath::Abs(Distance));
		break;
	}

	InOutResult.BlurRadius = BlurAmount * MaxBlurRadius;
	
	// 模糊的文字可以降低不透明度来增强效果
	if (BlurAmount > 0.0f)
	{
		InOutResult.Opacity *= FMath::Lerp(1.0f, BlurOpacity, BlurAmount);
	}
}

inline float FDreamLyricBlurEffect::CalculateBlurAmount(float Distance) const
{
	float AbsDistance = FMath::Abs(Distance);
	
	if (AbsDistance > BlurRange)
	{
		return 1.0f; // 最大模糊
	}

	// 线性增加模糊
	return AbsDistance / BlurRange;
}