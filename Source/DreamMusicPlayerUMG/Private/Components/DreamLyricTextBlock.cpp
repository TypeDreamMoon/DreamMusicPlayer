// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DreamLyricTextBlock.h"
#include "Widgets/Text/SDreamLyricTextBlock.h"
#include "Effect/DreamLyricColorEffect.h"
#include "Effect/DreamLyricScaleEffect.h"
#include "Effect/DreamLyricBlurEffect.h"
#include "Effect/DreamLyricGlowEffect.h"

#define LOCTEXT_NAMESPACE "DreamLyricTextBlock"

UDreamLyricTextBlock::UDreamLyricTextBlock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Text = FText::FromString(TEXT("歌词文本"));
	Font = FCoreStyle::GetDefaultFontStyle("Regular", 24);
	DisplayMode = EDreamLyricDisplayMode::Character;
	WrapWidth = 800.0f;
	LineSpacing = 1.2f;
	CurrentProgress = 0.0f;

	// 颜色效果默认值
	bEnableColorEffect = true;
	PlayedColor = FLinearColor(0.2f, 1.0f, 0.5f);
	UnplayedColor = FLinearColor(0.5f, 0.5f, 0.5f);
	TransitionColor = FLinearColor(1.0f, 0.8f, 0.0f);
	ColorTransitionRange = 2;

	// 缩放效果默认值
	bEnableScaleEffect = false;
	ScaleMode = EDreamLyricScaleMode::Current;
	MaxScale = 1.3f;
	MinScale = 1.0f;
	ScaleRange = 3;
	ScaleAnimationSpeed = 1.0f;

	// 模糊效果默认值
	bEnableBlurEffect = false;
	BlurMode = EDreamLyricBlurMode::Unplayed;
	MaxBlurRadius = 2.0f;
	BlurRange = 5;
	BlurOpacity = 0.3f;

	// 发光效果默认值
	bEnableGlowEffect = false;
	GlowColor = FLinearColor::Yellow;
	GlowIntensity = 0.5f;
	GlowRadius = 2.0f;
	GlowLayers = 3;
}

void UDreamLyricTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyLyricBlock.IsValid())
	{
		MyLyricBlock->SetText(Text);
		ApplyEffectSettings();
	}
}

void UDreamLyricTextBlock::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyLyricBlock.Reset();
	ColorEffect.Reset();
	ScaleEffect.Reset();
	BlurEffect.Reset();
	GlowEffect.Reset();
}

TSharedRef<SWidget> UDreamLyricTextBlock::RebuildWidget()
{
	MyLyricBlock = SNew(SDreamLyricTextBlock)
		.Text(Text)
		.Font(Font)
		.WrapWidth(WrapWidth)
		.LineSpacing(LineSpacing)
		.DisplayMode(DisplayMode);

	// 获取效果引用
	if (MyLyricBlock.IsValid())
	{
		ColorEffect = MyLyricBlock->GetEffectManager().GetEffect<FDreamLyricColorEffect>();
		ScaleEffect = MyLyricBlock->GetEffectManager().GetEffect<FDreamLyricScaleEffect>();
		BlurEffect = MyLyricBlock->GetEffectManager().GetEffect<FDreamLyricBlurEffect>();
		GlowEffect = MyLyricBlock->GetEffectManager().GetEffect<FDreamLyricGlowEffect>();

		ApplyEffectSettings();
	}

	return MyLyricBlock.ToSharedRef();
}

void UDreamLyricTextBlock::SetProgress(float InProgress)
{
	CurrentProgress = FMath::Clamp(InProgress, 0.0f, 1.0f);

	if (MyLyricBlock.IsValid())
	{
		MyLyricBlock->SetProgress(CurrentProgress);
	}
}

void UDreamLyricTextBlock::SetLyricText(FText InText)
{
	Text = InText;

	if (MyLyricBlock.IsValid())
	{
		MyLyricBlock->SetText(Text);
	}
}

void UDreamLyricTextBlock::SetDisplayMode(EDreamLyricDisplayMode InMode)
{
	DisplayMode = InMode;

	if (MyLyricBlock.IsValid())
	{
		MyLyricBlock->SetDisplayMode(InMode);
	}
}

void UDreamLyricTextBlock::SetColorEffectEnabled(bool bEnabled)
{
	bEnableColorEffect = bEnabled;

	if (ColorEffect.IsValid())
	{
		ColorEffect->SetEnabled(bEnabled);
	}
}

void UDreamLyricTextBlock::SetScaleEffectEnabled(bool bEnabled)
{
	bEnableScaleEffect = bEnabled;

	if (ScaleEffect.IsValid())
	{
		ScaleEffect->SetEnabled(bEnabled);
	}
}

void UDreamLyricTextBlock::SetBlurEffectEnabled(bool bEnabled)
{
	bEnableBlurEffect = bEnabled;

	if (BlurEffect.IsValid())
	{
		BlurEffect->SetEnabled(bEnabled);
	}
}

void UDreamLyricTextBlock::SetGlowEffectEnabled(bool bEnabled)
{
	bEnableGlowEffect = bEnabled;

	if (GlowEffect.IsValid())
	{
		GlowEffect->SetEnabled(bEnabled);
	}
}

void UDreamLyricTextBlock::SetScaleMode(EDreamLyricScaleMode InMode)
{
	ScaleMode = InMode;

	if (ScaleEffect.IsValid())
	{
		ScaleEffect->SetScaleMode(InMode);
	}
}

void UDreamLyricTextBlock::SetBlurMode(EDreamLyricBlurMode InMode)
{
	BlurMode = InMode;

	if (BlurEffect.IsValid())
	{
		BlurEffect->SetBlurMode(InMode);
	}
}

void UDreamLyricTextBlock::ApplyEffectSettings()
{
	// 应用颜色效果设置
	if (ColorEffect.IsValid())
	{
		ColorEffect->SetEnabled(bEnableColorEffect);
		ColorEffect->SetPlayedColor(PlayedColor);
		ColorEffect->SetUnplayedColor(UnplayedColor);
		ColorEffect->SetTransitionColor(TransitionColor);
		ColorEffect->SetTransitionRange(ColorTransitionRange);
	}

	// 应用缩放效果设置
	if (ScaleEffect.IsValid())
	{
		ScaleEffect->SetEnabled(bEnableScaleEffect);
		ScaleEffect->SetScaleMode(ScaleMode);
		ScaleEffect->SetMaxScale(MaxScale);
		ScaleEffect->SetMinScale(MinScale);
		ScaleEffect->SetScaleRange(ScaleRange);
		ScaleEffect->SetAnimationSpeed(ScaleAnimationSpeed);
	}

	// 应用模糊效果设置
	if (BlurEffect.IsValid())
	{
		BlurEffect->SetEnabled(bEnableBlurEffect);
		BlurEffect->SetBlurMode(BlurMode);
		BlurEffect->SetMaxBlurRadius(MaxBlurRadius);
		BlurEffect->SetBlurRange(BlurRange);
		BlurEffect->SetBlurOpacity(BlurOpacity);
	}

	// 应用发光效果设置
	if (GlowEffect.IsValid())
	{
		GlowEffect->SetEnabled(bEnableGlowEffect);
		GlowEffect->SetGlowColor(GlowColor);
		GlowEffect->SetGlowIntensity(GlowIntensity);
		GlowEffect->SetGlowRadius(GlowRadius);
		GlowEffect->SetGlowLayers(GlowLayers);
	}
}

#if WITH_EDITOR
const FText UDreamLyricTextBlock::GetPaletteCategory()
{
	return LOCTEXT("DreamLyricText", "Dream Widget");
}
#endif

#undef LOCTEXT_NAMESPACE
