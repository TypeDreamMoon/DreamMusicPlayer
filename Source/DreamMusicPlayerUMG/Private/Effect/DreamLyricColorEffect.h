// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDreamLyricEffect.h"

/**
 * 颜色渐变效果
 */
class DREAMMUSICPLAYERUMG_API FDreamLyricColorEffect : public IDreamLyricEffect
{
public:
	FDreamLyricColorEffect();

	virtual void ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const override;
	virtual bool IsEnabled() const override { return bEnabled; }
	virtual void SetEnabled(bool bInEnabled) override { bEnabled = bInEnabled; }
	virtual int32 GetPriority() const override { return 10; } // 颜色优先级高

	// 配置
	void SetPlayedColor(const FLinearColor& InColor) { PlayedColor = InColor; }
	void SetUnplayedColor(const FLinearColor& InColor) { UnplayedColor = InColor; }
	void SetTransitionColor(const FLinearColor& InColor) { TransitionColor = InColor; }
	void SetTransitionRange(int32 InRange) { TransitionRange = InRange; }

	const FLinearColor& GetPlayedColor() const { return PlayedColor; }
	const FLinearColor& GetUnplayedColor() const { return UnplayedColor; }
	const FLinearColor& GetTransitionColor() const { return TransitionColor; }

private:
	FLinearColor CalculateUnitColor(int32 UnitIndex, float TargetIndex) const;

	bool bEnabled;
	FLinearColor PlayedColor;
	FLinearColor UnplayedColor;
	FLinearColor TransitionColor;
	int32 TransitionRange; // 渐变过渡字符数
};

// Implementation
inline FDreamLyricColorEffect::FDreamLyricColorEffect()
	: bEnabled(true)
	, PlayedColor(0.2f, 1.0f, 0.5f, 1.0f)
	, UnplayedColor(0.5f, 0.5f, 0.5f, 1.0f)
	, TransitionColor(FLinearColor::Yellow)
	, TransitionRange(2)
{
}

inline void FDreamLyricColorEffect::ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const
{
	if (!bEnabled || Context.TotalUnits == 0)
	{
		return;
	}

	float TargetIndex = Context.CurrentProgress * Context.TotalUnits;
	InOutResult.Color = CalculateUnitColor(Context.Unit.GlobalIndex, TargetIndex);
}

inline FLinearColor FDreamLyricColorEffect::CalculateUnitColor(int32 UnitIndex, float TargetIndex) const
{
	float Distance = UnitIndex - TargetIndex;

	// 当前正在播放的单元
	if (FMath::Abs(Distance) < 0.5f)
	{
		return TransitionColor;
	}
	// 已播放完的单元
	else if (Distance < -TransitionRange)
	{
		return PlayedColor;
	}
	// 未播放的单元
	else if (Distance > TransitionRange)
	{
		return UnplayedColor;
	}
	// 渐变过渡区域
	else
	{
		if (Distance < 0)
		{
			float Alpha = 1.0f - FMath::Abs(Distance) / TransitionRange;
			return FMath::Lerp(TransitionColor, PlayedColor, Alpha);
		}
		else
		{
			float Alpha = Distance / TransitionRange;
			return FMath::Lerp(TransitionColor, UnplayedColor, Alpha);
		}
	}
}