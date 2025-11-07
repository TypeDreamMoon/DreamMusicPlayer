#include "LyricAsset.h"

ULyricAsset::ULyricAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

int64 FLyricTimeSpan::ToTotalMilliseconds() const
{
	return static_cast<int64>(Hours) * 3600000LL +
		   static_cast<int64>(Minutes) * 60000LL +
		   static_cast<int64>(Seconds) * 1000LL +
		   static_cast<int64>(Milliseconds);
}

