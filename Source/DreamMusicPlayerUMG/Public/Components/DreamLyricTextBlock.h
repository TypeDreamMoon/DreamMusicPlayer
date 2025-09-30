// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Effect/DreamLyricScaleEffect.h"
#include "Effect/DreamLyricBlurEffect.h"
#include "DreamLyricTextBlock.generated.h"

enum class EDreamLyricDisplayMode : uint8;
class SDreamLyricTextBlock;
class FDreamLyricColorEffect;
class FDreamLyricScaleEffect;
class FDreamLyricBlurEffect;
class FDreamLyricGlowEffect;

/**
 * 重构版 UMG 歌词控件 - 支持模块化效果系统
 */
UCLASS()
class DREAMMUSICPLAYERUMG_API UDreamLyricTextBlock : public UUserWidget
{
	GENERATED_BODY()

public:
	UDreamLyricTextBlock(const FObjectInitializer& ObjectInitializer);

	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	// ============ 基础属性 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Content")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Appearance")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Display")
	EDreamLyricDisplayMode DisplayMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Layout", meta = (ClampMin = "0"))
	float WrapWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Layout", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float LineSpacing;

	// ============ 颜色效果 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Color")
	bool bEnableColorEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Color", meta = (EditCondition = "bEnableColorEffect"))
	FLinearColor PlayedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Color", meta = (EditCondition = "bEnableColorEffect"))
	FLinearColor UnplayedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Color", meta = (EditCondition = "bEnableColorEffect"))
	FLinearColor TransitionColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Color", meta = (EditCondition = "bEnableColorEffect", ClampMin = "0", ClampMax = "10"))
	int32 ColorTransitionRange;

	// ============ 缩放效果 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale")
	bool bEnableScaleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale", meta = (EditCondition = "bEnableScaleEffect"))
	EDreamLyricScaleMode ScaleMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale", meta = (EditCondition = "bEnableScaleEffect", ClampMin = "1.0", ClampMax = "3.0"))
	float MaxScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale", meta = (EditCondition = "bEnableScaleEffect", ClampMin = "0.1", ClampMax = "1.0"))
	float MinScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale", meta = (EditCondition = "bEnableScaleEffect", ClampMin = "1", ClampMax = "10"))
	int32 ScaleRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Scale", meta = (EditCondition = "bEnableScaleEffect", ClampMin = "0.1", ClampMax = "5.0"))
	float ScaleAnimationSpeed;

	// ============ 模糊效果 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Blur")
	bool bEnableBlurEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Blur", meta = (EditCondition = "bEnableBlurEffect"))
	EDreamLyricBlurMode BlurMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Blur", meta = (EditCondition = "bEnableBlurEffect", ClampMin = "0.0", ClampMax = "5.0"))
	float MaxBlurRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Blur", meta = (EditCondition = "bEnableBlurEffect", ClampMin = "1", ClampMax = "10"))
	int32 BlurRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Blur", meta = (EditCondition = "bEnableBlurEffect", ClampMin = "0.0", ClampMax = "1.0"))
	float BlurOpacity;

	// ============ 发光效果 ============

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Glow")
	bool bEnableGlowEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Glow", meta = (EditCondition = "bEnableGlowEffect"))
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Glow", meta = (EditCondition = "bEnableGlowEffect", ClampMin = "0.0", ClampMax = "1.0"))
	float GlowIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Glow", meta = (EditCondition = "bEnableGlowEffect", ClampMin = "0.0", ClampMax = "5.0"))
	float GlowRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Effect|Glow", meta = (EditCondition = "bEnableGlowEffect", ClampMin = "1", ClampMax = "5"))
	int32 GlowLayers;

	// ============ 方法 ============

	UFUNCTION(BlueprintCallable, Category = "Lyric")
	void SetProgress(float InProgress);

	UFUNCTION(BlueprintCallable, Category = "Lyric")
	void SetLyricText(FText InText);

	UFUNCTION(BlueprintCallable, Category = "Lyric")
	void SetDisplayMode(EDreamLyricDisplayMode InMode);

	UFUNCTION(BlueprintPure, Category = "Lyric")
	float GetProgress() const { return CurrentProgress; }

	// ============ 运行时效果配置 ============

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetColorEffectEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetScaleEffectEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetBlurEffectEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetGlowEffectEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetScaleMode(EDreamLyricScaleMode InMode);

	UFUNCTION(BlueprintCallable, Category = "Lyric|Effect")
	void SetBlurMode(EDreamLyricBlurMode InMode);

private:
	void ApplyEffectSettings();

	TSharedPtr<SDreamLyricTextBlock> MyLyricBlock;
	float CurrentProgress;

	// 效果引用缓存
	TSharedPtr<FDreamLyricColorEffect> ColorEffect;
	TSharedPtr<FDreamLyricScaleEffect> ScaleEffect;
	TSharedPtr<FDreamLyricBlurEffect> BlurEffect;
	TSharedPtr<FDreamLyricGlowEffect> GlowEffect;
};
