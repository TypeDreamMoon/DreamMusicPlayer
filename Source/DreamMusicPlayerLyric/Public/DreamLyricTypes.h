#pragma once

#include "CoreMinimal.h"
#include "DreamMusicTimestamp.h"
#include "dlp/File.hpp"
#include "DreamLyricTypes.generated.h"

/**
 * @brief 歌词文本角色枚举
 * 用于区分原歌词、音译、翻译等不同类型的文本
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDreamMusicLyricTextRole : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Lyric = 1 UMETA(DisplayName = "Lyric"),
	Romanization = 2 UMETA(DisplayName = "Romanization"),
	Translation = 3 UMETA(DisplayName = "Translation")
};


USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamMusicLyricWord
{
	GENERATED_BODY()

public:
	FDreamMusicLyricWord();

	FDreamMusicLyricWord(FDreamMusicTimestamp InStartTimestamp, FDreamMusicTimestamp InEndTimestamp, FString InContent);

	FDreamMusicLyricWord(const FString& InContent, const FDreamMusicTimestamp& InStartTimestamp, const FDreamMusicTimestamp& InEndTimestamp = FDreamMusicTimestamp());

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicTimestamp StartTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicTimestamp EndTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FString Content;

	/**
	 * @brief 获取单词持续时间（毫秒）
	 */
	int64 GetDurationMilliseconds() const;

	/**
	 * @brief 判断时间点是否在单词时间范围内
	 */
	bool IsTimeInRange(const FDreamMusicTimestamp& Time) const;

	/**
	 * @brief 判断是否为空单词
	 */
	bool IsEmpty() const;
	
	bool operator==(const FDreamMusicLyricWord&) const;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamMusicLyric
{
	GENERATED_BODY()

public:
	FDreamMusicLyric()
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicTimestamp StartTimestamp = FDreamMusicTimestamp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicTimestamp EndTimestamp = FDreamMusicTimestamp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDreamMusicLyricWord> WordTimings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEmptyLine = false;

public:
	bool operator==(const FDreamMusicLyric& Target) const;
	bool operator==(const FDreamMusicTimestamp& Target) const;
	bool operator!=(const FDreamMusicLyric& Target) const;

	inline bool IsNotEmpty() const
	{
		return StartTimestamp.ToMilliseconds() > 0 || !Content.IsEmpty();
	}

	inline bool IsEmpty() const
	{
		return !IsNotEmpty();
	}

	inline bool IsWordsEmpty() const
	{
		return WordTimings.IsEmpty();
	}

	static FDreamMusicLyric EMPTY()
	{
		static FDreamMusicLyric Lyric;
		Lyric.bIsEmptyLine = true;
		return Lyric;
	}
};

USTRUCT(BlueprintType)
struct FDreamMusicLyricProgress
{
	GENERATED_BODY()

public:
	FDreamMusicLyricProgress()
		: CurrentWordIndex(-1)
		  , LineProgress(0.0f)
		  , bIsActive(false)
	{
	}

	FDreamMusicLyricProgress(int32 InCurrentWordIndex, float InLineProgress, bool InIsActive, const FDreamMusicLyricWord& InCurrentWord)
		: CurrentWordIndex(InCurrentWordIndex)
		  , LineProgress(InLineProgress)
		  , bIsActive(InIsActive)
		  , CurrentWord(InCurrentWord)
	{
	}

	FDreamMusicLyricProgress(float InLineProgress)
		: CurrentWordIndex(-1)
		  , LineProgress(InLineProgress)
		  , bIsActive(false)
	{
	}

	// Current word index being played (-1 if none)
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentWordIndex;

	// Progress within the current line (0.0 to 1.0)
	UPROPERTY(BlueprintReadOnly)
	float LineProgress;

	// Whether this line is currently active
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive;

	// Current word being played (if any)
	UPROPERTY(BlueprintReadOnly)
	FDreamMusicLyricWord CurrentWord;
};

/**
 * Lyric Source Type
 */
UENUM(BlueprintType)
enum class EDreamMuiscPlayerLyricSourceType : uint8
{
	// 使用歌词资产
	Asset UMETA(DisplayName = "Asset"),

	// 使用网络下载
	URL UMETA(DisplayName = "URL"),

	// 使用QQ音乐 酷狗 网易云 Lrclib进行搜索
	// Search UMETA(DisplayName = "Network Search (Unsupport)"),

	// 使用文件
	File UMETA(DisplayName = "File")
};

/**
 * Lyric File Type
 */
UENUM(BlueprintType)
enum class EDreamMusicPlayerLyricType : uint8
{
	/**
	 * LyRiC Format
	 */
	LRC UMETA(DisplayName = "LRC"),
	/**
	 * ESLyric Word-by-word Format
	 */
	ESLyric UMETA(DisplayName = "ESLyric"),
	/**
	 * SubRip
	 */
	SRT UMETA(DisplayName = "SRT"),
	/**
	 * ASS Subtitle Format
	 * @see https://fileinfo.com/extension/ass
	 */
	ASS UMETA(DisplayName = "ASS"),

	// --------------------- dlp library not support ---------------------

	/**
	 * NetEase Cloud Music Word-by-word Format
	 */
	// YRC UMETA(DisplayName = "YRC"),
	/**
	 * QQ Music Word-by-word Format
	 */
	// QRC UMETA(DisplayName = "QRC"),
	/**
	 * Lyricify Syllable Word-by-word Format
	 */
	// LYS UMETA(DisplayName = "LYS"),
	/**
	 * TTML Lyric Format
	 */
	// TTML UMETA(DisplayName = "TTML (Unsupport)"),
	/**
	 * Dream Music Player Unify lyrics
	 */
	// DMPUL UMETA(DisplayName = "DMPUL (Unsupport)"),
};


/**
 * @brief 歌词行结构
 * 
 * 表示一行歌词，可以包含多个单词（用于逐词显示）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamMusicLyricLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line")
	EDreamMusicLyricTextRole Role = EDreamMusicLyricTextRole::Lyric;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line", meta = (MultiLine = true))
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Line", meta = (TitleProperty = "Content"))
	TArray<FDreamMusicLyricWord> Words;

	FDreamMusicLyricLine() = default;

	FDreamMusicLyricLine(const FString& InText, EDreamMusicLyricTextRole InRole = EDreamMusicLyricTextRole::Lyric)
		: Role(InRole), Text(InText)
	{
	}

	/**
	 * @brief 获取行的开始时间（从第一个单词）
	 */
	FDreamMusicTimestamp GetStartTimestamp() const;

	/**
	 * @brief 获取行的结束时间（从最后一个单词）
	 */
	FDreamMusicTimestamp GetEndTimestamp() const;

	/**
	 * @brief 判断时间点是否在行的时间范围内
	 */
	bool IsTimeInRange(const FDreamMusicTimestamp& Time) const;

	/**
	 * @brief 判断是否为空行
	 */
	bool IsEmpty() const;

	/**
	 * @brief 获取行的总单词数
	 */
	int32 GetWordCount() const { return Words.Num(); }

	bool operator==(const FDreamMusicLyricLine& Target) const;
};

/**
 * @brief 歌词组结构
 * 
 * 表示一个时间点的歌词组，可以包含多行（如原歌词、音译、翻译等）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamMusicLyricGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FDreamMusicTimestamp StartTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FDreamMusicTimestamp EndTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (TitleProperty = "Text"))
	TArray<FDreamMusicLyricLine> Lines;

	FDreamMusicLyricGroup() = default;

	FDreamMusicLyricGroup(const FDreamMusicTimestamp& InTimestamp)
		: StartTimestamp(InTimestamp)
	{
	}

	/**
	 * @brief 获取指定角色的行
	 */
	FDreamMusicLyricLine* GetLineByRole(EDreamMusicLyricTextRole Role);

	/**
	 * @brief 获取指定角色的行（常量版本）
	 */
	const FDreamMusicLyricLine* GetLineByRole(EDreamMusicLyricTextRole Role) const;

	/**
	 * @brief 获取主歌词行（通常是 Lyric 角色）
	 */
	FDreamMusicLyricLine* GetMainLyricLine();

	/**
	 * @brief 判断时间点是否匹配此组
	 */
	bool IsTimeMatch(const FDreamMusicTimestamp& Time, float ToleranceSeconds = 0.1f) const;

	/**
	 * @brief 判断是否为空组
	 */
	bool IsEmpty() const;

	/**
	 * @brief 获取组的总行数
	 */
	int32 GetLineCount() const { return Lines.Num(); }

	bool IsRomanizationWordsEmpty() const;

	bool IsWordsEmpty() const;

	const FDreamMusicLyricLine* operator[](EDreamMusicLyricTextRole InRole) const;
	bool operator==(const FDreamMusicLyricGroup& Other) const;
};

/**
 * @brief 歌词元数据结构
 * 
 * 存储歌词文件的元数据信息（如标题、艺术家、专辑等）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamMusicLyricMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	TMap<FString, FString> Items;

	FDreamMusicLyricMetadata() = default;

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
 * @brief 歌词解析选项配置
 * 
 * 用于配置歌词解析器的行为，包括分组规则等
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamLyricParserOptions
{
	GENERATED_BODY()

	/**
	 * @brief 分组序列 - 指定歌词行的分组顺序（按角色）
	 * 例如：[Lyric, Translation] 表示先按原歌词分组，再按翻译分组
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parser Options", meta = (Bitflags))
	TArray<EDreamMusicLyricTextRole> GroupingSequence;

	/**
	 * @brief 回退角色 - 当无法匹配序列中的角色时使用的默认角色
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parser Options")
	EDreamMusicLyricTextRole FallbackRole = EDreamMusicLyricTextRole::Lyric;

	/**
	 * @brief 使用默认配置
	 */
	static FDreamLyricParserOptions GetDefault()
	{
		FDreamLyricParserOptions Options;
		Options.GroupingSequence.Add(EDreamMusicLyricTextRole::Lyric);
		Options.FallbackRole = EDreamMusicLyricTextRole::Lyric;
		return Options;
	}

	FDreamLyricParserOptions()
	{
		GroupingSequence.Add(EDreamMusicLyricTextRole::Lyric);
		FallbackRole = EDreamMusicLyricTextRole::Lyric;
	}

	dlp::File::FLyricGroupRoleOption ToLibraryType() const;
};
