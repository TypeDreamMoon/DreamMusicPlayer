#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LyricAsset.generated.h"

/**
 * @brief 歌词时间跨度结构
 * 
 * 用于表示歌词的时间戳，支持小时、分钟、秒和毫秒
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricTimeSpan
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0"))
	int32 Hours = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0", ClampMax = "59"))
	int32 Minutes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0", ClampMax = "59"))
	int32 Seconds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0", ClampMax = "999"))
	int32 Milliseconds = 0;

	FLyricTimeSpan() = default;
	FLyricTimeSpan(int32 H, int32 M, int32 S, int32 MS) 
		: Hours(H), Minutes(M), Seconds(S), Milliseconds(MS) 
	{
		Normalize();
	}

	/**
	 * @brief 转换为总毫秒数
	 */
	int64 ToTotalMilliseconds() const;

	/**
	 * @brief 转换为总秒数（浮点数）
	 */
	float ToSeconds() const;

	/**
	 * @brief 转换为总分钟数（浮点数）
	 */
	float ToMinutes() const;

	/**
	 * @brief 从总毫秒数创建时间跨度
	 */
	static FLyricTimeSpan FromMilliseconds(int64 TotalMilliseconds);

	/**
	 * @brief 从总秒数创建时间跨度
	 */
	static FLyricTimeSpan FromSeconds(float TotalSeconds);

	/**
	 * @brief 规范化时间值（确保分钟、秒、毫秒在有效范围内）
	 */
	void Normalize();

	/**
	 * @brief 格式化为字符串（HH:MM:SS.mmm 或 MM:SS.mmm）
	 */
	FString ToString(bool bIncludeHours = false, int32 FractionalDigits = 3) const;

	/**
	 * @brief 判断是否为零时间
	 */
	bool IsZero() const;

	// 操作符重载
	bool operator==(const FLyricTimeSpan& Other) const;
	bool operator!=(const FLyricTimeSpan& Other) const;
	bool operator<(const FLyricTimeSpan& Other) const;
	bool operator<=(const FLyricTimeSpan& Other) const;
	bool operator>(const FLyricTimeSpan& Other) const;
	bool operator>=(const FLyricTimeSpan& Other) const;
	FLyricTimeSpan operator+(const FLyricTimeSpan& Other) const;
	FLyricTimeSpan operator-(const FLyricTimeSpan& Other) const;
};

/**
 * @brief 歌词文本角色枚举
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ELyricTextRole : uint8
{
	None = 0			UMETA(DisplayName = "None"),
	Lyric = 1			UMETA(DisplayName = "Lyric"),
	Romanization = 2	UMETA(DisplayName = "Romanization"),
	Translation = 4		UMETA(DisplayName = "Translation")
};

/**
 * @brief 歌词单词结构
 * 
 * 表示歌词中的一个单词，包含文本、时间戳和角色信息
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricWord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Word", meta = (MultiLine = true))
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Word")
	FLyricTimeSpan StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Word")
	FLyricTimeSpan EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Word")
	bool bHasEndTime = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Word")
	ELyricTextRole Role = ELyricTextRole::Lyric;

	FLyricWord() = default;
	FLyricWord(const FString& InText, const FLyricTimeSpan& InStartTime, const FLyricTimeSpan& InEndTime = FLyricTimeSpan())
		: Text(InText), StartTime(InStartTime), EndTime(InEndTime), bHasEndTime(false), Role(ELyricTextRole::Lyric)
	{
		if (InEndTime.ToTotalMilliseconds() > 0)
		{
			bHasEndTime = true;
		}
	}

	/**
	 * @brief 获取单词持续时间（毫秒）
	 */
	int64 GetDurationMilliseconds() const;

	/**
	 * @brief 判断时间点是否在单词时间范围内
	 */
	bool IsTimeInRange(const FLyricTimeSpan& Time) const;

	/**
	 * @brief 判断是否为空单词
	 */
	bool IsEmpty() const;
};

/**
 * @brief 歌词行结构
 * 
 * 表示一行歌词，可以包含多个单词（用于逐词显示）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line")
	ELyricTextRole Role = ELyricTextRole::Lyric;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line", meta = (MultiLine = true))
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line", meta = (TitleProperty = "Text"))
	TArray<FLyricWord> Words;

	FLyricLine() = default;
	FLyricLine(const FString& InText, ELyricTextRole InRole = ELyricTextRole::Lyric)
		: Role(InRole), Text(InText) {}

	/**
	 * @brief 获取行的开始时间（从第一个单词）
	 */
	FLyricTimeSpan GetStartTime() const;

	/**
	 * @brief 获取行的结束时间（从最后一个单词）
	 */
	FLyricTimeSpan GetEndTime() const;

	/**
	 * @brief 判断时间点是否在行的时间范围内
	 */
	bool IsTimeInRange(const FLyricTimeSpan& Time) const;

	/**
	 * @brief 判断是否为空行
	 */
	bool IsEmpty() const;

	/**
	 * @brief 获取行的总单词数
	 */
	int32 GetWordCount() const { return Words.Num(); }
};

/**
 * @brief 歌词组结构
 * 
 * 表示一个时间点的歌词组，可以包含多行（如原歌词、音译、翻译等）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FLyricTimeSpan Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (TitleProperty = "Text"))
	TArray<FLyricLine> Lines;

	FLyricGroup() = default;
	FLyricGroup(const FLyricTimeSpan& InTimestamp)
		: Timestamp(InTimestamp) {}

	/**
	 * @brief 获取指定角色的行
	 */
	FLyricLine* GetLineByRole(ELyricTextRole Role);

	/**
	 * @brief 获取指定角色的行（常量版本）
	 */
	const FLyricLine* GetLineByRole(ELyricTextRole Role) const;

	/**
	 * @brief 获取主歌词行（通常是 Lyric 角色）
	 */
	FLyricLine* GetMainLyricLine();

	/**
	 * @brief 判断时间点是否匹配此组
	 */
	bool IsTimeMatch(const FLyricTimeSpan& Time, float ToleranceSeconds = 0.1f) const;

	/**
	 * @brief 判断是否为空组
	 */
	bool IsEmpty() const;

	/**
	 * @brief 获取组的总行数
	 */
	int32 GetLineCount() const { return Lines.Num(); }
};

/**
 * @brief 歌词元数据结构
 * 
 * 存储歌词文件的元数据信息（如标题、艺术家、专辑等）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	TMap<FString, FString> Items;

	FLyricMetadata() = default;

	/**
	 * @brief 获取元数据值
	 */
	FString GetValue(const FString& Key, const FString& DefaultValue = TEXT("")) const;

	/**
	 * @brief 设置元数据值
	 */
	void SetValue(const FString& Key, const FString& Value);

	/**
	 * @brief 检查是否包含指定键
	 */
	bool HasKey(const FString& Key) const;

	/**
	 * @brief 获取常用元数据（标题、艺术家、专辑）
	 */
	FString GetTitle() const { return GetValue(TEXT("ti"), GetValue(TEXT("title"))); }

	FString GetArtist() const { return GetValue(TEXT("ar"), GetValue(TEXT("artist"))); }

	FString GetAlbum() const { return GetValue(TEXT("al"), GetValue(TEXT("album"))); }

	FString GetCreator() const { return GetValue(TEXT("by"), GetValue(TEXT("creator"))); }
};

/**
 * @brief 歌词资产统计信息
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FLyricAssetStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalGroups = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalLines = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalWords = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FLyricTimeSpan StartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	FLyricTimeSpan EndTime;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	bool bHasWordTimings = false;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	bool bHasMultipleRoles = false;
};

/**
 * @brief 歌词资产类
 * 
 * 这是歌词文件在 UE 中的资产表示，包含完整的歌词数据、元数据和工具方法
 */
UCLASS(BlueprintType, meta = (DisplayName = "Lyric Asset"))
class DREAMMUSICPLAYERLYRIC_API ULyricAsset : public UObject
{
	GENERATED_BODY()

public:
	ULyricAsset(const FObjectInitializer& ObjectInitializer);

	// ========== 核心数据 ==========
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Data", meta = (DisplayName = "Metadata"))
	FLyricMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Data", meta = (DisplayName = "Groups"))
	TArray<FLyricGroup> Groups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyric|Info", meta = (DisplayName = "Source File Name"))
	FString SourceFileName;

	// ========== 查询方法 ==========

	/**
	 * @brief 根据时间查找对应的歌词组
	 * 
	 * @param Time 查询时间
	 * @param ToleranceSeconds 容差（秒）
	 * @return 找到的歌词组索引，如果未找到返回 INDEX_NONE
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	int32 FindGroupByTime(const FLyricTimeSpan& Time, float ToleranceSeconds = 0.1f) const;

	/**
	 * @brief 根据时间获取对应的歌词组
	 * 
	 * @param Time 查询时间
	 * @param ToleranceSeconds 容差（秒）
	 * @return 找到的歌词组指针，如果未找到返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	FLyricGroup GetGroupByTime(const FLyricTimeSpan& Time, float ToleranceSeconds = 0.1f);
	
	/**
	 * @brief 获取指定角色的所有行
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Query")
	TArray<FLyricLine> GetLinesByRole(ELyricTextRole Role) const;

	// ========== 统计信息 ==========

	/**
	 * @brief 获取资产统计信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	FLyricAssetStatistics GetStatistics() const;

	/**
	 * @brief 获取总时长（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	float GetTotalDurationSeconds() const;

	/**
	 * @brief 获取总时长（时间跨度）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	FLyricTimeSpan GetTotalDuration() const;

	/**
	 * @brief 获取组数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetGroupCount() const { return Groups.Num(); }

	/**
	 * @brief 获取总行数
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetTotalLineCount() const;

	/**
	 * @brief 获取总单词数
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Statistics")
	int32 GetTotalWordCount() const;

	// ========== 工具方法 ==========

	/**
	 * @brief 对组按时间戳排序
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	void SortGroupsByTime();

	/**
	 * @brief 验证资产数据的有效性
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool Validate() const;

	/**
	 * @brief 获取验证错误列表
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	TArray<FString> GetValidationErrors() const;

	/**
	 * @brief 清空所有数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	void Clear();

	/**
	 * @brief 判断资产是否为空
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool IsEmpty() const;

	/**
	 * @brief 检查是否包含逐词时间信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool HasWordTimings() const;

	/**
	 * @brief 检查是否包含多种角色（原歌词、音译、翻译等）
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Tools")
	bool HasMultipleRoles() const;

	// ========== 元数据便捷访问 ==========

	/**
	 * @brief 获取标题
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetTitle() const { return Metadata.GetTitle(); }

	/**
	 * @brief 获取艺术家
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetArtist() const { return Metadata.GetArtist(); }

	/**
	 * @brief 获取专辑
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetAlbum() const { return Metadata.GetAlbum(); }

	/**
	 * @brief 获取创建者
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyric|Metadata")
	FString GetCreator() const { return Metadata.GetCreator(); }

#if WITH_EDITOR
	// ========== 编辑器专用方法 ==========

	/**
	 * @brief 获取资产描述（用于内容浏览器）
	 */
	virtual FString GetDesc() override;
#endif
};
