// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDreamLyricEffect.h"

/**
 * 歌词效果管理器
 * 负责管理和组合多个效果
 */
class DREAMMUSICPLAYERUMG_API FDreamLyricEffectManager
{
public:
	FDreamLyricEffectManager();
	~FDreamLyricEffectManager() = default;

	/** 添加效果 */
	void AddEffect(TSharedPtr<IDreamLyricEffect> Effect);

	/** 移除效果 */
	void RemoveEffect(TSharedPtr<IDreamLyricEffect> Effect);

	/** 清除所有效果 */
	void ClearEffects();

	/** 获取所有效果 */
	const TArray<TSharedPtr<IDreamLyricEffect>>& GetEffects() const { return Effects; }

	/** 根据类型获取效果 */
	template <typename EffectType>
	TSharedPtr<EffectType> GetEffect() const;

	/** 应用所有启用的效果 */
	FDreamLyricEffectResult ApplyEffects(const FDreamLyricEffectContext& Context) const;

	/** 对效果进行排序（按优先级） */
	void SortEffects();

private:
	TArray<TSharedPtr<IDreamLyricEffect>> Effects;
	mutable bool bNeedsSorting;
};

// Implementation
inline FDreamLyricEffectManager::FDreamLyricEffectManager()
	: bNeedsSorting(false)
{
}

inline void FDreamLyricEffectManager::AddEffect(TSharedPtr<IDreamLyricEffect> Effect)
{
	if (Effect.IsValid())
	{
		Effects.Add(Effect);
		bNeedsSorting = true;
	}
}

inline void FDreamLyricEffectManager::RemoveEffect(TSharedPtr<IDreamLyricEffect> Effect)
{
	Effects.Remove(Effect);
}

inline void FDreamLyricEffectManager::ClearEffects()
{
	Effects.Empty();
}

template <typename EffectType>
inline TSharedPtr<EffectType> FDreamLyricEffectManager::GetEffect() const
{
	for (const TSharedPtr<IDreamLyricEffect>& Effect : Effects)
	{
		TSharedPtr<EffectType> CastedEffect = StaticCastSharedPtr<EffectType>(Effect);
		if (CastedEffect.IsValid())
		{
			return CastedEffect;
		}
	}
	return nullptr;
}

inline FDreamLyricEffectResult FDreamLyricEffectManager::ApplyEffects(const FDreamLyricEffectContext& Context) const
{
	// 确保效果按优先级排序
	if (bNeedsSorting)
	{
		const_cast<FDreamLyricEffectManager*>(this)->SortEffects();
	}

	FDreamLyricEffectResult Result;

	// 按优先级依次应用每个启用的效果
	for (const TSharedPtr<IDreamLyricEffect>& Effect : Effects)
	{
		if (Effect.IsValid() && Effect->IsEnabled())
		{
			Effect->ApplyEffect(Context, Result);
		}
	}

	return Result;
}

inline void FDreamLyricEffectManager::SortEffects()
{
	Effects.Sort([](const TSharedPtr<IDreamLyricEffect>& A, const TSharedPtr<IDreamLyricEffect>& B)
	{
		return A->GetPriority() < B->GetPriority();
	});
	bNeedsSorting = false;
}
