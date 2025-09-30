// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FSlateWindowElementList;
class FWidgetStyle;
struct FGeometry;
struct FSlateFontInfo;

/**
 * 显示单元数据结构
 */
struct FDreamLyricDisplayUnit
{
	FString Content; // 内容
	FVector2D Position; // 位置
	FVector2D Size; // 尺寸
	int32 GlobalIndex; // 全局索引
	int32 LineIndex; // 所在行索引
	bool bIsSpace; // 是否是空格

	FDreamLyricDisplayUnit()
		: GlobalIndex(0)
		  , LineIndex(0)
		  , bIsSpace(false)
	{
	}
};

/**
 * 效果渲染上下文
 */
struct FDreamLyricEffectContext
{
	const FDreamLyricDisplayUnit& Unit;
	const FGeometry& Geometry;
	const FSlateFontInfo& Font;
	const FWidgetStyle& WidgetStyle;
	float CurrentProgress;
	int32 TotalUnits;
	int32 LayerId;

	FDreamLyricEffectContext(
		const FDreamLyricDisplayUnit& InUnit,
		const FGeometry& InGeometry,
		const FSlateFontInfo& InFont,
		const FWidgetStyle& InWidgetStyle,
		float InProgress,
		int32 InTotalUnits,
		int32 InLayerId)
		: Unit(InUnit)
		  , Geometry(InGeometry)
		  , Font(InFont)
		  , WidgetStyle(InWidgetStyle)
		  , CurrentProgress(InProgress)
		  , TotalUnits(InTotalUnits)
		  , LayerId(InLayerId)
	{
	}
};

/**
 * 效果应用结果
 */
struct FDreamLyricEffectResult
{
	FLinearColor Color;
	FVector2D Scale;
	float Opacity;
	float BlurRadius;
	FVector2D Offset;
	bool bShouldRender;

	FDreamLyricEffectResult()
		: Color(FLinearColor::White)
		  , Scale(FVector2D::UnitVector)
		  , Opacity(1.0f)
		  , BlurRadius(0.0f)
		  , Offset(FVector2D::ZeroVector)
		  , bShouldRender(true)
	{
	}
};

/**
 * 歌词效果接口
 */
class DREAMMUSICPLAYERUMG_API IDreamLyricEffect
{
public:
	virtual ~IDreamLyricEffect() = default;

	/** 应用效果到渲染结果 */
	virtual void ApplyEffect(const FDreamLyricEffectContext& Context, FDreamLyricEffectResult& InOutResult) const = 0;

	/** 效果是否启用 */
	virtual bool IsEnabled() const = 0;

	/** 设置效果启用状态 */
	virtual void SetEnabled(bool bInEnabled) = 0;

	/** 效果优先级（数值越小越先执行） */
	virtual int32 GetPriority() const { return 100; }
};
