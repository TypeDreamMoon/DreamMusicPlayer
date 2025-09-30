// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/Text/SDreamLyricTextBlock.h"
#include "SlateOptMacros.h"
#include "Fonts/FontMeasure.h"
#include "Effect/DreamLyricColorEffect.h"
#include "Effect/DreamLyricScaleEffect.h"
#include "Effect/DreamLyricBlurEffect.h"
#include "Effect/DreamLyricGlowEffect.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SDreamLyricTextBlock)

void SDreamLyricTextBlock::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, Text, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, Font, EInvalidateWidgetReason::Layout);
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION(AttributeInitializer, WrapWidth, EInvalidateWidgetReason::Layout);
}

SDreamLyricTextBlock::SDreamLyricTextBlock()
	: Text(*this)
	  , Font(*this)
	  , WrapWidth(*this)
	  , LineSpacing(1.2f)
	  , CurrentProgress(0.0f)
	  , DisplayMode(EDreamLyricDisplayMode::Character)
	  , bNeedsRebuild(true)
{
	SetCanTick(false);
	bCanSupportFocus = false;

	// 初始化默认效果
	EffectManager.AddEffect(MakeShared<FDreamLyricColorEffect>());
	EffectManager.AddEffect(MakeShared<FDreamLyricScaleEffect>());
	EffectManager.AddEffect(MakeShared<FDreamLyricBlurEffect>());
	EffectManager.AddEffect(MakeShared<FDreamLyricGlowEffect>());
}

void SDreamLyricTextBlock::Construct(const FArguments& InArgs)
{
	Text.Assign(*this, InArgs._Text);
	Font.Assign(*this, InArgs._Font);
	WrapWidth.Assign(*this, InArgs._WrapWidth);
	LineSpacing = InArgs._LineSpacing;
	DisplayMode = InArgs._DisplayMode;
}

int32 SDreamLyricTextBlock::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	RebuildLayout();

	const int32 TotalUnits = DisplayUnits.Num();
	if (TotalUnits == 0)
	{
		return LayerId;
	}

	const FSlateFontInfo& CurrentFont = Font.Get();
	int32 CurrentLayer = LayerId;

	// 获取发光效果（如果存在）
	TSharedPtr<FDreamLyricGlowEffect> GlowEffect = EffectManager.GetEffect<FDreamLyricGlowEffect>();

	// 渲染所有单元
	for (int32 i = 0; i < TotalUnits; ++i)
	{
		const FDreamLyricDisplayUnit& Unit = DisplayUnits[i];

		// 创建效果上下文
		FDreamLyricEffectContext Context(
			Unit,
			AllottedGeometry,
			CurrentFont,
			InWidgetStyle,
			CurrentProgress,
			TotalUnits,
			CurrentLayer
		);

		// 应用所有效果
		FDreamLyricEffectResult EffectResult = EffectManager.ApplyEffects(Context);

		if (!EffectResult.bShouldRender)
		{
			continue;
		}

		// 先渲染发光效果
		if (GlowEffect.IsValid())
		{
			GlowEffect->RenderGlowLayers(OutDrawElements, Context, EffectResult);
		}

		// 如果有模糊效果，使用特殊渲染
		if (EffectResult.BlurRadius > 0.0f)
		{
			RenderBlurEffect(OutDrawElements, CurrentLayer + 1, AllottedGeometry,
			                 Unit, EffectResult, InWidgetStyle);
		}
		else
		{
			// 普通渲染
			RenderUnit(OutDrawElements, CurrentLayer + 1, AllottedGeometry,
			           Unit, EffectResult, InWidgetStyle);
		}
	}

	return CurrentLayer + 3;
}

FVector2D SDreamLyricTextBlock::ComputeDesiredSize(float X) const
{
	RebuildLayout();
	return CachedSize;
}

void SDreamLyricTextBlock::SetProgress(float InProgress)
{
	CurrentProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SDreamLyricTextBlock::SetText(const FText& InText)
{
	Text.Set(*this, InText);
	bNeedsRebuild = true;
	Invalidate(EInvalidateWidgetReason::Layout);
}

void SDreamLyricTextBlock::SetDisplayMode(EDreamLyricDisplayMode InMode)
{
	if (DisplayMode != InMode)
	{
		DisplayMode = InMode;
		bNeedsRebuild = true;
		Invalidate(EInvalidateWidgetReason::Layout);
	}
}

void SDreamLyricTextBlock::RebuildLayout() const
{
	if (!bNeedsRebuild && !DisplayUnits.IsEmpty())
	{
		return;
	}

	DisplayUnits.Empty();

	const FString TextString = Text.Get().ToString();
	if (TextString.IsEmpty())
	{
		CachedSize = FVector2D::ZeroVector;
		bNeedsRebuild = false;
		return;
	}

	const FSlateFontInfo& CurrentFont = Font.Get();
	const float MaxWidth = WrapWidth.Get();

	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	FVector2D SpaceSize = FontMeasure->Measure(FText::FromString(TEXT(" ")), CurrentFont);
	float LineHeight = SpaceSize.Y * LineSpacing;

	switch (DisplayMode)
	{
	case EDreamLyricDisplayMode::Character:
		BuildCharacterLayout(TextString, CurrentFont, MaxWidth, LineHeight);
		break;
	case EDreamLyricDisplayMode::Word:
		BuildWordLayout(TextString, CurrentFont, MaxWidth, LineHeight);
		break;
	case EDreamLyricDisplayMode::Line:
		BuildLineLayout(TextString, CurrentFont, MaxWidth, LineHeight);
		break;
	}

	bNeedsRebuild = false;
}

void SDreamLyricTextBlock::BuildCharacterLayout(
	const FString& TextString,
	const FSlateFontInfo& CurrentFont,
	float MaxWidth,
	float LineHeight) const
{
	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	float CurrentX = 0.0f;
	float CurrentY = 0.0f;
	float MaxLineWidth = 0.0f;
	int32 GlobalIndex = 0;
	int32 LineIndex = 0;

	for (int32 i = 0; i < TextString.Len(); ++i)
	{
		TCHAR Char = TextString[i];

		if (Char == '\n')
		{
			MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
			CurrentX = 0.0f;
			CurrentY += LineHeight;
			LineIndex++;
			GlobalIndex++;
			continue;
		}

		FString CharStr = TextString.Mid(i, 1);
		FVector2D CharSize = FontMeasure->Measure(FText::FromString(CharStr), CurrentFont);

		if (CurrentX + CharSize.X > MaxWidth && CurrentX > 0.0f)
		{
			MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
			CurrentX = 0.0f;
			CurrentY += LineHeight;
			LineIndex++;
		}

		FDreamLyricDisplayUnit Unit;
		Unit.Content = CharStr;
		Unit.Position = FVector2D(CurrentX, CurrentY);
		Unit.Size = CharSize;
		Unit.GlobalIndex = GlobalIndex;
		Unit.LineIndex = LineIndex;
		Unit.bIsSpace = (Char == ' ');

		DisplayUnits.Add(Unit);

		CurrentX += CharSize.X;
		GlobalIndex++;
	}

	MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
	CachedSize = FVector2D(MaxLineWidth, CurrentY + LineHeight);
}

void SDreamLyricTextBlock::BuildWordLayout(
	const FString& TextString,
	const FSlateFontInfo& CurrentFont,
	float MaxWidth,
	float LineHeight) const
{
	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	TArray<FString> Words;
	TArray<bool> IsNewLine;
	FString CurrentWord;

	for (int32 i = 0; i < TextString.Len(); ++i)
	{
		TCHAR Char = TextString[i];

		if (Char == ' ' || Char == '\n')
		{
			if (!CurrentWord.IsEmpty())
			{
				Words.Add(CurrentWord);
				IsNewLine.Add(Char == '\n');
				CurrentWord.Empty();
			}

			if (Char == ' ')
			{
				Words.Add(TEXT(" "));
				IsNewLine.Add(false);
			}
		}
		else
		{
			CurrentWord.AppendChar(Char);
		}
	}

	if (!CurrentWord.IsEmpty())
	{
		Words.Add(CurrentWord);
		IsNewLine.Add(false);
	}

	float CurrentX = 0.0f;
	float CurrentY = 0.0f;
	float MaxLineWidth = 0.0f;
	int32 LineIndex = 0;

	for (int32 i = 0; i < Words.Num(); ++i)
	{
		const FString& Word = Words[i];
		FVector2D WordSize = FontMeasure->Measure(FText::FromString(Word), CurrentFont);

		if (CurrentX + WordSize.X > MaxWidth && CurrentX > 0.0f && Word != TEXT(" "))
		{
			MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
			CurrentX = 0.0f;
			CurrentY += LineHeight;
			LineIndex++;
		}

		FDreamLyricDisplayUnit Unit;
		Unit.Content = Word;
		Unit.Position = FVector2D(CurrentX, CurrentY);
		Unit.Size = WordSize;
		Unit.GlobalIndex = i;
		Unit.LineIndex = LineIndex;
		Unit.bIsSpace = (Word == TEXT(" "));

		DisplayUnits.Add(Unit);

		CurrentX += WordSize.X;

		if (IsNewLine[i])
		{
			MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
			CurrentX = 0.0f;
			CurrentY += LineHeight;
			LineIndex++;
		}
	}

	MaxLineWidth = FMath::Max(MaxLineWidth, CurrentX);
	CachedSize = FVector2D(MaxLineWidth, CurrentY + LineHeight);
}

void SDreamLyricTextBlock::BuildLineLayout(
	const FString& TextString,
	const FSlateFontInfo& CurrentFont,
	float MaxWidth,
	float LineHeight) const
{
	TSharedRef<FSlateFontMeasure> FontMeasure =
		FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	TArray<FString> Lines;
	TextString.ParseIntoArray(Lines, TEXT("\n"), false);

	float CurrentY = 0.0f;
	float MaxLineWidth = 0.0f;

	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		const FString& Line = Lines[i];
		if (Line.IsEmpty()) continue;

		FVector2D LineSize = FontMeasure->Measure(FText::FromString(Line), CurrentFont);

		FDreamLyricDisplayUnit Unit;
		Unit.Content = Line;
		Unit.Position = FVector2D(0.0f, CurrentY);
		Unit.Size = LineSize;
		Unit.GlobalIndex = i;
		Unit.LineIndex = i;
		Unit.bIsSpace = false;

		DisplayUnits.Add(Unit);

		MaxLineWidth = FMath::Max(MaxLineWidth, LineSize.X);
		CurrentY += LineHeight;
	}

	CachedSize = FVector2D(MaxLineWidth, CurrentY);
}

void SDreamLyricTextBlock::RenderUnit(
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FGeometry& AllottedGeometry,
	const FDreamLyricDisplayUnit& Unit,
	const FDreamLyricEffectResult& EffectResult,
	const FWidgetStyle& InWidgetStyle) const
{
	const FSlateFontInfo& CurrentFont = Font.Get();
	FText UnitText = FText::FromString(Unit.Content);

	// 计算缩放后的位置（从中心缩放）
	FVector2D ScaledSize = Unit.Size * EffectResult.Scale;
	FVector2D SizeOffset = (ScaledSize - Unit.Size) * 0.5f;
	FVector2D FinalPosition = Unit.Position - EffectResult.Offset;

	FSlateLayoutTransform UnitTransform(FinalPosition);

	FLinearColor FinalColor = EffectResult.Color;
	FinalColor.A *= EffectResult.Opacity;

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(
			FVector2f(ScaledSize),
			UnitTransform
		),
		UnitText,
		CurrentFont,
		ESlateDrawEffect::None,
		InWidgetStyle.GetColorAndOpacityTint() * FinalColor
	);
}

void SDreamLyricTextBlock::RenderBlurEffect(
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FGeometry& AllottedGeometry,
	const FDreamLyricDisplayUnit& Unit,
	const FDreamLyricEffectResult& EffectResult,
	const FWidgetStyle& InWidgetStyle) const
{
	const FSlateFontInfo& CurrentFont = Font.Get();
	FText UnitText = FText::FromString(Unit.Content);

	FVector2D ScaledSize = Unit.Size * EffectResult.Scale;
	FVector2D SizeOffset = (ScaledSize - Unit.Size) * 0.5f;
	FVector2D BasePosition = Unit.Position - SizeOffset + EffectResult.Offset;

	FLinearColor FinalColor = EffectResult.Color;
	FinalColor.A *= EffectResult.Opacity * 0.3f; // 模糊层透明度

	// 模拟模糊：绘制多个偏移的副本
	const int32 BlurSamples = FMath::CeilToInt(EffectResult.BlurRadius);
	for (int32 x = -BlurSamples; x <= BlurSamples; ++x)
	{
		for (int32 y = -BlurSamples; y <= BlurSamples; ++y)
		{
			if (x == 0 && y == 0) continue;

			FVector2D BlurOffset(x, y);
			FVector2D BlurPosition = BasePosition + BlurOffset;
			FSlateLayoutTransform BlurTransform(BlurPosition);

			FSlateDrawElement::MakeText(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(ScaledSize),
					BlurTransform
				),
				UnitText,
				CurrentFont,
				ESlateDrawEffect::None,
				InWidgetStyle.GetColorAndOpacityTint() * FinalColor
			);
		}
	}

	// 绘制主文本
	FinalColor.A = EffectResult.Opacity;
	FSlateLayoutTransform MainTransform(BasePosition);

	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(
			FVector2f(ScaledSize),
			MainTransform
		),
		UnitText,
		CurrentFont,
		ESlateDrawEffect::None,
		InWidgetStyle.GetColorAndOpacityTint() * FinalColor
	);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
