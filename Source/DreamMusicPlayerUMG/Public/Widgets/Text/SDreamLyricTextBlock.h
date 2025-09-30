// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "IDreamLyricEffect.h"
#include "DreamLyricEffectManager.h"

UENUM(BlueprintType)
enum class EDreamLyricDisplayMode : uint8
{
	/** 逐字符显示 */
	Character,
	/** 逐词显示 */
	Word,
	/** 逐行显示 */
	Line
};

/**
 * 重构版歌词文本块 - 使用模块化效果系统
 */
class DREAMMUSICPLAYERUMG_API SDreamLyricTextBlock : public SLeafWidget
{
	SLATE_DECLARE_WIDGET(SDreamLyricTextBlock, SLeafWidget)

public:
	SLATE_BEGIN_ARGS(SDreamLyricTextBlock)
			: _Text(FText::GetEmpty())
			  , _Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
			  , _WrapWidth(800.0f)
			  , _LineSpacing(1.2f)
			  , _DisplayMode(EDreamLyricDisplayMode::Character)
		{
		}

		SLATE_ATTRIBUTE(FText, Text)
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)
		SLATE_ATTRIBUTE(float, WrapWidth)
		SLATE_ARGUMENT(float, LineSpacing)
		SLATE_ARGUMENT(EDreamLyricDisplayMode, DisplayMode)
	SLATE_END_ARGS()

	SDreamLyricTextBlock();

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override;

	/** 设置播放进度 */
	void SetProgress(float InProgress);

	/** 设置文本 */
	void SetText(const FText& InText);

	/** 设置显示模式 */
	void SetDisplayMode(EDreamLyricDisplayMode InMode);

	/** 获取效果管理器 */
	FDreamLyricEffectManager& GetEffectManager() { return EffectManager; }
	const FDreamLyricEffectManager& GetEffectManager() const { return EffectManager; }

private:
	/** 重建布局 */
	void RebuildLayout() const;

	/** 按字符模式构建布局 */
	void BuildCharacterLayout(const FString& TextString, const FSlateFontInfo& CurrentFont,
	                          float MaxWidth, float LineHeight) const;

	/** 按词模式构建布局 */
	void BuildWordLayout(const FString& TextString, const FSlateFontInfo& CurrentFont,
	                     float MaxWidth, float LineHeight) const;

	/** 按行模式构建布局 */
	void BuildLineLayout(const FString& TextString, const FSlateFontInfo& CurrentFont,
	                     float MaxWidth, float LineHeight) const;

	/** 渲染单个显示单元 */
	void RenderUnit(
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FGeometry& AllottedGeometry,
		const FDreamLyricDisplayUnit& Unit,
		const FDreamLyricEffectResult& EffectResult,
		const FWidgetStyle& InWidgetStyle) const;

	/** 渲染模糊效果（通过多次绘制模拟） */
	void RenderBlurEffect(
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FGeometry& AllottedGeometry,
		const FDreamLyricDisplayUnit& Unit,
		const FDreamLyricEffectResult& EffectResult,
		const FWidgetStyle& InWidgetStyle) const;

	// Slate 属性
	TSlateAttribute<FText> Text;
	TSlateAttribute<FSlateFontInfo> Font;
	TSlateAttribute<float> WrapWidth;

	// 参数
	float LineSpacing;
	float CurrentProgress;
	EDreamLyricDisplayMode DisplayMode;

	// 效果系统
	FDreamLyricEffectManager EffectManager;

	// 缓存
	mutable TArray<FDreamLyricDisplayUnit> DisplayUnits;
	mutable FVector2D CachedSize;
	mutable bool bNeedsRebuild;
};
