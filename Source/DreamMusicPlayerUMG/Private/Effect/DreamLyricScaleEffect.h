// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDreamLyricEffect.h"

/**
 * 缩放动画效果模式
 */
UENUM(BlueprintType)
enum class EDreamLyricScaleMode : uint8
{
	/** 当前播放字符放大 */
	Current,
	
	/** 波纹式放大（当前最大，周围递减） */
	Ripple,
	
	/** 弹跳效果 */
	Bounce,
	
	/** 脉冲效果 */
	Pulse
};

/**
 * 缩放动画效果
 */
class DREAMMUSICPLAYERUMG_API FDreamLyricScaleEffect : public IDreamLyricEffect
{
public:
	FDreamLyricScaleEffect();

	virtual void ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const override;
	virtual bool IsEnabled() const override { return bEnabled; }
	virtual void SetEnabled(bool bInEnabled) override { bEnabled = bInEnabled; }
	virtual int32 GetPriority() const override { return 30; } // 缩放在颜色之后

	// 配置
	void SetScaleMode(EDreamLyricScaleMode InMode) { ScaleMode = InMode; }
	void SetMaxScale(float InScale) { MaxScale = FMath::Max(1.0f, InScale); }
	void SetMinScale(float InScale) { MinScale = FMath::Clamp(InScale, 0.1f, 1.0f); }
	void SetScaleRange(int32 InRange) { ScaleRange = FMath::Max(1, InRange); }
	void SetAnimationSpeed(float InSpeed) { AnimationSpeed = FMath::Max(0.1f, InSpeed); }

	EDreamLyricScaleMode GetScaleMode() const { return ScaleMode; }
	float GetMaxScale() const { return MaxScale; }

private:
	float CalculateScaleForCurrent(float Distance) const;
	float CalculateScaleForRipple(float Distance) const;
	float CalculateScaleForBounce(float Distance, float Time) const;
	float CalculateScaleForPulse(float Distance, float Time) const;

	bool bEnabled;
	EDreamLyricScaleMode ScaleMode;
	float MaxScale;        // 最大缩放倍数
	float MinScale;        // 最小缩放倍数
	int32 ScaleRange;      // 缩放影响范围
	float AnimationSpeed;  // 动画速度
};

// Implementation
inline FDreamLyricScaleEffect::FDreamLyricScaleEffect()
	: bEnabled(true)
	, ScaleMode(EDreamLyricScaleMode::Current)
	, MaxScale(1.3f)
	, MinScale(1.0f)
	, ScaleRange(3)
	, AnimationSpeed(1.0f)
{
}

inline void FDreamLyricScaleEffect::ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const
{
	if (!bEnabled || Context.TotalUnits == 0)
	{
		return;
	}

	float TargetIndex = Context.CurrentProgress * Context.TotalUnits;
	float Distance = Context.Unit.GlobalIndex - TargetIndex;
	float Scale = 1.0f;

	// 使用进度作为时间参数（用于动画）
	float Time = Context.CurrentProgress * 10.0f * AnimationSpeed;

	switch (ScaleMode)
	{
	case EDreamLyricScaleMode::Current:
		Scale = CalculateScaleForCurrent(Distance);
		break;
	case EDreamLyricScaleMode::Ripple:
		Scale = CalculateScaleForRipple(Distance);
		break;
	case EDreamLyricScaleMode::Bounce:
		Scale = CalculateScaleForBounce(Distance, Time);
		break;
	case EDreamLyricScaleMode::Pulse:
		Scale = CalculateScaleForPulse(Distance, Time);
		break;
	}

	InOutResult.Scale = FVector2D(Scale, Scale);
}

inline float FDreamLyricScaleEffect::CalculateScaleForCurrent(float Distance) const
{
	if (FMath::Abs(Distance) < 0.5f)
	{
		return MaxScale;
	}
	return MinScale;
}

inline float FDreamLyricScaleEffect::CalculateScaleForRipple(float Distance) const
{
	float AbsDistance = FMath::Abs(Distance);
	
	if (AbsDistance > ScaleRange)
	{
		return MinScale;
	}

	// 线性衰减
	float Alpha = 1.0f - (AbsDistance / ScaleRange);
	return FMath::Lerp(MinScale, MaxScale, Alpha);
}

inline float FDreamLyricScaleEffect::CalculateScaleForBounce(float Distance, float Time) const
{
	if (FMath::Abs(Distance) > 0.5f)
	{
		return MinScale;
	}

	// 使用 sin 函数创建弹跳效果
	float BounceValue = FMath::Abs(FMath::Sin(Time * PI));
	return FMath::Lerp(MinScale, MaxScale, BounceValue);
}

inline float FDreamLyricScaleEffect::CalculateScaleForPulse(float Distance, float Time) const
{
	float AbsDistance = FMath::Abs(Distance);
	
	if (AbsDistance > ScaleRange)
	{
		return MinScale;
	}

	// 脉冲效果：结合距离和时间
	float DistanceAlpha = 1.0f - (AbsDistance / ScaleRange);
	float PulseValue = (FMath::Sin(Time * PI * 2.0f) * 0.5f + 0.5f);
	float Scale = FMath::Lerp(MinScale, MaxScale, DistanceAlpha * PulseValue);
	
	return Scale;
}