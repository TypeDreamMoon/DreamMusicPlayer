#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "DreamMusicTimestamp.generated.h"


USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicTimestamp
{
	GENERATED_BODY()

public:
	FDreamMusicTimestamp() : Hours(0), Minute(0), Seconds(0), Millisecond(0)
	{
	};

	FDreamMusicTimestamp(std::chrono::duration<double> Duration);

	FDreamMusicTimestamp(float InSeconds);

	FDreamMusicTimestamp(int InMinute, int InSeconds, int InMillisecond) :
		Minute(InMinute), Seconds(InSeconds), Millisecond(InMillisecond)
	{
	}

	FDreamMusicTimestamp(int InHours, int InMinute, int InSeconds, int InMillisecond)
		: Hours(InHours), Minute(InMinute), Seconds(InSeconds), Millisecond(InMillisecond)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Hours = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Minute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Seconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Millisecond = 0;

public:
	bool operator==(const FDreamMusicTimestamp& Target) const;
	bool operator>=(const FDreamMusicTimestamp& Target) const;
	bool operator>(const FDreamMusicTimestamp& Target) const;
	bool operator<=(const FDreamMusicTimestamp& Target) const;
	bool operator<(const FDreamMusicTimestamp& Target) const;
	const std::strong_ordering operator<=>(const FDreamMusicTimestamp& Target) const;

	bool IsApproximatelyEqual(const FDreamMusicTimestamp& Target, int ToleranceMilliseconds) const;

	FString ToString() const
	{
		return FString::Printf(TEXT("%02d:%02d:%02d.%03d"), Hours, Minute, Seconds, Millisecond);
	}

	inline const FDreamMusicTimestamp* FromSeconds(float InSeconds);
	inline float ToSeconds() const;
	inline int ToMilliseconds() const;

	/**
	 * @brief 转换为总毫秒数（int64，用于大时间跨度）
	 */
	int64 ToTotalMilliseconds() const;

	/**
	 * @brief 从总毫秒数创建时间戳
	 */
	static FDreamMusicTimestamp FromTotalMilliseconds(int64 TotalMilliseconds);

	/**
	 * @brief 从总秒数创建时间戳（静态方法）
	 */
	static FDreamMusicTimestamp FromSecondsStatic(float TotalSeconds);

	/**
	 * @brief 规范化时间值（确保分钟、秒、毫秒在有效范围内）
	 */
	void Normalize();

	/**
	 * @brief 格式化为字符串（HH:MM:SS.mmm 或 MM:SS.mmm）
	 */
	FString ToStringFormatted(bool bIncludeHours = false, int32 FractionalDigits = 3) const;

	/**
	 * @brief 判断是否为零时间
	 */
	bool IsZero() const;
	
	static FDreamMusicTimestamp Parse(const FString& TimestampStr);
	

	// 操作符重载（已存在，但添加减法）
	FDreamMusicTimestamp operator+(const FDreamMusicTimestamp& Other) const;
	FDreamMusicTimestamp operator-(const FDreamMusicTimestamp& Other) const;
};
