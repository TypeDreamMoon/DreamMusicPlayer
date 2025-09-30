// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDreamLyricEffect.h"
#include "Rendering/DrawElements.h"

/**
 * 发光效果
 */
class DREAMMUSICPLAYERUMG_API FDreamLyricGlowEffect : public IDreamLyricEffect
{
public:
	FDreamLyricGlowEffect();

	virtual void ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const override;
	virtual bool IsEnabled() const override { return bEnabled; }
	virtual void SetEnabled(bool bInEnabled) override { bEnabled = bInEnabled; }
	virtual int32 GetPriority() const override { return 50; } // 发光是装饰效果，最后应用

	// 配置
	void SetGlowColor(const FLinearColor& InColor) { GlowColor = InColor; }
	void SetGlowIntensity(float InIntensity) { GlowIntensity = FMath::Max(0.0f, InIntensity); }
	void SetGlowRadius(float InRadius) { GlowRadius = FMath::Max(0.0f, InRadius); }
	void SetGlowLayers(int32 InLayers) { GlowLayers = FMath::Clamp(InLayers, 1, 5); }

	const FLinearColor& GetGlowColor() const { return GlowColor; }
	float GetGlowIntensity() const { return GlowIntensity; }
	int32 GetGlowLayers() const { return GlowLayers; }
	float GetGlowRadius() const { return GlowRadius; }

	/** 绘制发光层（需要在主渲染前调用） */
	void RenderGlowLayers(
		FSlateWindowElementList& OutDrawElements,
		const FDreamLyricEffectContext& Context,
		const FDreamLyricEffectResult& Result) const;

	/** 判断当前单元是否应该发光 */
	bool ShouldGlow(const FDreamLyricEffectContext& Context) const;

private:
	bool bEnabled;
	FLinearColor GlowColor;
	float GlowIntensity;   // 发光强度
	float GlowRadius;      // 发光半径
	int32 GlowLayers;      // 发光层数
};

// Implementation
inline FDreamLyricGlowEffect::FDreamLyricGlowEffect()
	: bEnabled(false)
	, GlowColor(FLinearColor::Yellow)
	, GlowIntensity(0.5f)
	, GlowRadius(2.0f)
	, GlowLayers(3)
{
}

inline void FDreamLyricGlowEffect::ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const
{
	// 发光效果主要通过 RenderGlowLayers 实现
	// 这里可以标记需要发光的单元
}

inline bool FDreamLyricGlowEffect::ShouldGlow(const FDreamLyricEffectContext& Context) const
{
	if (!bEnabled || Context.Unit.bIsSpace || Context.TotalUnits == 0)
	{
		return false;
	}

	float TargetIndex = Context.CurrentProgress * Context.TotalUnits;
	float Distance = FMath::Abs(Context.Unit.GlobalIndex - TargetIndex);
	
	return Distance < 0.5f; // 当前播放的字符发光
}

inline void FDreamLyricGlowEffect::RenderGlowLayers(
	FSlateWindowElementList& OutDrawElements,
	const FDreamLyricEffectContext& Context,
	const FDreamLyricEffectResult& Result) const
{
	if (!ShouldGlow(Context))
	{
		return;
	}

	FText UnitText = FText::FromString(Context.Unit.Content);
	FLinearColor GlowColorWithAlpha = GlowColor;
	GlowColorWithAlpha.A = GlowIntensity;

	// 绘制多层发光效果
	for (int32 i = 0; i < GlowLayers; ++i)
	{
		float LayerRadius = GlowRadius * (i + 1) / GlowLayers;
		float LayerAlpha = GlowIntensity * (1.0f - static_cast<float>(i) / GlowLayers);

		// 在四个方向绘制发光
		const FVector2D Offsets[] = {
			FVector2D(LayerRadius, 0),
			FVector2D(-LayerRadius, 0),
			FVector2D(0, LayerRadius),
			FVector2D(0, -LayerRadius)
		};

		for (const FVector2D& Offset : Offsets)
		{
			FVector2D GlowPosition = Context.Unit.Position * Result.Scale + Offset;
			FSlateLayoutTransform GlowTransform(GlowPosition);

			FSlateDrawElement::MakeText(
				OutDrawElements,
				Context.LayerId,
				Context.Geometry.ToPaintGeometry(
					FVector2f(Context.Unit.Size * Result.Scale),
					GlowTransform
				),
				UnitText,
				Context.Font,
				ESlateDrawEffect::None,
				Context.WidgetStyle.GetColorAndOpacityTint() * FLinearColor(GlowColor.R, GlowColor.G, GlowColor.B, LayerAlpha)
			);
		}
	}
}