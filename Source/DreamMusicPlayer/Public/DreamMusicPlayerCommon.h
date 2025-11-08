#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetStringLibrary.h"
#include "DreamMusicPlayerCommon.generated.h"

class UDreamMusicPlayerExpansionData;
class UDreamMusicData;
class UConstantQNRT;
class ULoudnessNRT;

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayState : uint8
{
	EDMPPS_Stop = 0 UMETA(DisplayName = "Stop"),
	EDMPPS_Playing = 1 UMETA(DisplayName = "Playing"),
	EDMPPS_Paused = 2 UMETA(DisplayName = "Paused"),
};

UENUM(BlueprintType)
enum class EDreamMusicPlayerPlayMode : uint8
{
	EDMPPS_Normal = 0 UMETA(DisplayName = "Normal"),
	EDMPPS_Loop = 1 UMETA(DisplayName = "Loop"),
	EDMPPS_Random = 2 UMETA(DisplayName = "Random")
};

/**
 * Lyric File Type
 */
UENUM(BlueprintType)
enum class EDreamMusicPlayerLyricType : uint8
{
	// Asset
	Asset UMETA(DisplayName = "Asset"),
	// Network Stream
	Stream UMETA(DisplayName = "Stream [UnderDevelopment]"),
	// Three Types of LRC File
	LRC UMETA(DisplayName = "LRC"),
	/**
	 * [Sample] SRT
	 * 1
	 * 00:00:48,710 --> 00:00:58,770
	 * 風 触れる ホシの 願い
	 *
	 * 2
	 * 00:00:59,210 --> 00:01:08,690
	 * 見上げ 地に縛られている
	 */
	SRT UMETA(DisplayName = "SRT"),
	/**
	 * [Sample] ASS
	 * @see https://fileinfo.com/extension/ass
	 */
	ASS UMETA(DisplayName = "ASS"),
};

UENUM(BlueprintType)
enum class EDreamMusicPlayerLrcLyricType : uint8
{
	None UMETA(DisplayName = "None"),
	/**
	 * [Sample] Line By Line LRC Lyric
	 * [00:48.710]風 触れる ホシの 願い
	 * [00:59.210]見上げ 地に縛られている
	 */
	LineByLine UMETA(DisplayName = "LineByLine"),
	/**
	 * [Sample] Word By Word LRC Lyric
	 * [00:48.710]風 [00:50.560]触[00:51.060]れ[00:51.270]る [00:53.570]ホ[00:53.920]シ[00:54.800]の [00:56.620]願[00:57.270]い[00:58.770]
	 * [00:59.210]見[01:00.090]上[01:00.130]げ [01:02.610]地[01:03.030]に[01:03.640]縛[01:04.640]ら[01:06.010]れ[01:06.170]て[01:07.560]い[01:07.590]る[01:08.690]
	 */
	WordByWord UMETA(DisplayName = "WordByWord"),
	/**
	 * [Sample] ESLyric
	 * [00:48.710]<00:48.710>風 <00:50.560>触<00:51.060>れ<00:51.270>る <00:53.570>ホ<00:53.920>シ<00:54.800>の <00:56.620>願<00:57.270>い<00:58.770>
	 * [00:59.210]<00:59.210>見<01:00.090>上<01:00.130>げ <01:02.610>地<01:03.030>に<01:03.640>縛<01:04.640>ら<01:06.010>れ<01:06.170>て<01:07.560>い<01:07.590>る<01:08.690>
	 */
	ESLyric UMETA(DisplayName = "ESLyric"),
};

UENUM(BlueprintType)
enum class EDreamMusicPlayerLyricParseLineType: uint8
{
	Romanization_Lyric_Translation UMETA(DisplayName = "Romanization-Lyric-Translation"),
	Romanization_Translation_Lyric UMETA(DisplayName = "Romanization-Translation-Lyric"),
	Translation_Romanization_Lyric UMETA(DisplayName = "Translation-Romanization-Lyric"),
	Translation_Lyric_Romanization UMETA(DisplayName = "Translation-Lyric-Romanization"),
	Lyric_Romanization_Translation UMETA(DisplayName = "Lyric-Romanization-Translation"),
	Romanization_Lyric UMETA(DisplayName = "Romanization-Lyric"),
	Lyric_Romanization UMETA(DisplayName = "Lyric-Romanization"),
	Translation_Lyric UMETA(DisplayName = "Translation-Lyric"),
	Lyric_Translation UMETA(DisplayName = "Lyric-Translation"),
	Lyric_Only UMETA(DisplayName = "Lyric-Only"),
};

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
struct DREAMMUSICPLAYER_API FDreamMusicLyricTimestamp
{
	GENERATED_BODY()

public:
	FDreamMusicLyricTimestamp() : Hours(0), Minute(0), Seconds(0), Millisecond(0)
	{
	};

	FDreamMusicLyricTimestamp(float InSeconds);

	FDreamMusicLyricTimestamp(int InMinute, int InSeconds, int InMillisecond) :
		Minute(InMinute), Seconds(InSeconds), Millisecond(InMillisecond)
	{
	}

	FDreamMusicLyricTimestamp(int InHours, int InMinute, int InSeconds, int InMillisecond)
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
	bool operator==(const FDreamMusicLyricTimestamp& Target) const;
	bool operator>=(const FDreamMusicLyricTimestamp& Target) const;
	bool operator>(const FDreamMusicLyricTimestamp& Target) const;
	bool operator<=(const FDreamMusicLyricTimestamp& Target) const;
	bool operator<(const FDreamMusicLyricTimestamp& Target) const;

	bool IsApproximatelyEqual(const FDreamMusicLyricTimestamp& Target, int ToleranceMilliseconds) const;

	static FDreamMusicLyricTimestamp Parse(const FString& TimestampStr)
	{
		FDreamMusicLyricTimestamp Ts;

		FString TimeString = TimestampStr;

		// Try HH:MM:SS,mmm or HH:MM:SS.mmm (SRT/ASS Format)
		{
			FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2}):(\\d{2})[,.](\\d{3}))"));
			FRegexMatcher RegexMatcher(RegexPattern, TimeString);

			if (RegexMatcher.FindNext())
			{
				Ts.Hours = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
				Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
				Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2));
				Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(3));
				return Ts;
			}
		}

		// Try MM:SS.mmm (LRC Format)
		{
			FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2}):(\\d{2})[.](\\d{3}))"));
			FRegexMatcher RegexMatcher(RegexPattern, TimeString);
			if (RegexMatcher.FindNext())
			{
				Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
				Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
				Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2));
				return Ts;
			}
		}

		// Try MM:SS.mm (LRC Format with 2-digit milliseconds)
		{
			FRegexPattern RegexPattern(TEXT("((\\d{1,2}):(\\d{2})\\.(\\d{2}))"));
			FRegexMatcher RegexMatcher(RegexPattern, TimeString);

			if (RegexMatcher.FindNext())
			{
				Ts.Minute = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(0));
				Ts.Seconds = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(1));
				Ts.Millisecond = UKismetStringLibrary::Conv_StringToInt(RegexMatcher.GetCaptureGroup(2)) * 10; // Convert to 3-digit milliseconds
			}
		}

		return Ts;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%02d:%02d:%02d.%03d"), Hours, Minute, Seconds, Millisecond);
	}

	inline const FDreamMusicLyricTimestamp* FromSeconds(float InSeconds);
	inline float ToSeconds() const;
	inline int ToMilliseconds() const;

	/**
	 * @brief 转换为总毫秒数（int64，用于大时间跨度）
	 */
	int64 ToTotalMilliseconds() const;

	/**
	 * @brief 从总毫秒数创建时间戳
	 */
	static FDreamMusicLyricTimestamp FromTotalMilliseconds(int64 TotalMilliseconds);

	/**
	 * @brief 从总秒数创建时间戳（静态方法）
	 */
	static FDreamMusicLyricTimestamp FromSecondsStatic(float TotalSeconds);

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

	// 操作符重载（已存在，但添加减法）
	FDreamMusicLyricTimestamp operator+(const FDreamMusicLyricTimestamp& Other) const;
	FDreamMusicLyricTimestamp operator-(const FDreamMusicLyricTimestamp& Other) const;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicLyricWord
{
	GENERATED_BODY()

public:
	FDreamMusicLyricWord();

	FDreamMusicLyricWord(FDreamMusicLyricTimestamp InStartTimestamp, FDreamMusicLyricTimestamp InEndTimestamp, FString InContent);

	FDreamMusicLyricWord(const FString& InContent, const FDreamMusicLyricTimestamp& InStartTimestamp, const FDreamMusicLyricTimestamp& InEndTimestamp = FDreamMusicLyricTimestamp());

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp StartTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp EndTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FString Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasEndTimestamp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDreamMusicLyricTextRole Role;

	/**
	 * @brief 获取单词持续时间（毫秒）
	 */
	int64 GetDurationMilliseconds() const;

	/**
	 * @brief 判断时间点是否在单词时间范围内
	 */
	bool IsTimeInRange(const FDreamMusicLyricTimestamp& Time) const;

	/**
	 * @brief 判断是否为空单词
	 */
	bool IsEmpty() const;
};

USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicLyric
{
	GENERATED_BODY()

public:
	FDreamMusicLyric()
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp StartTimestamp = FDreamMusicLyricTimestamp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicLyricTimestamp EndTimestamp = FDreamMusicLyricTimestamp();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Content;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Translate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Romanization;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDreamMusicLyricWord> RomanizationWordTimings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDreamMusicLyricWord> WordTimings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEmptyLine = false;

public:
	bool operator==(const FDreamMusicLyric& Target) const;
	bool operator==(const FDreamMusicLyricTimestamp& Target) const;
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

	inline bool IsRomanizationWordsEmpty() const
	{
		return RomanizationWordTimings.IsEmpty();
	}

	static FDreamMusicLyric EMPTY()
	{
		static FDreamMusicLyric Lyric;
		Lyric.bIsEmptyLine = true;
		return Lyric;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("Content: %s Translation: %s Romanization: %s Start: %s End: %s WordTimings: %d RomanizationTimings: %d"),
		                       *Content,
		                       *Translate,
		                       *Romanization,
		                       *StartTimestamp.ToString(),
		                       *EndTimestamp.ToString(),
		                       WordTimings.Num(),
		                       RomanizationWordTimings.Num());
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

USTRUCT(BlueprintType)
struct FDreamMusicInformation
{
	GENERATED_BODY()

public:
	FDreamMusicInformation()
	{
	};

	FDreamMusicInformation(FString InTitle, FString InArtist, FString InAlbum, TObjectPtr<UTexture2D> InCover,
	                       FString InGenre) :
		Title(InTitle), Artist(InArtist), Album(InAlbum), Cover(InCover),
		Genre(InGenre)
	{
	}

public:
	//歌曲标题
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title;

	//歌曲作者
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Artist;

	//歌曲专辑
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Album;

	//歌曲封面
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Cover;

	//歌曲流派
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Genre;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicInformation& Target) const;
};

// 歌曲数据
USTRUCT(BlueprintType)
struct FDreamMusicInformationData
{
	GENERATED_BODY()

public:
	FDreamMusicInformationData()
	{
	};

public:
	// 音乐资源软对象引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundWave> Music;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicInformationData& Target) const;
};

// 歌曲数据表
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicDataStruct
{
	GENERATED_BODY()

public:
	FDreamMusicDataStruct() : Information(FDreamMusicInformation()), Data(FDreamMusicInformationData())
	{
	};

	FDreamMusicDataStruct(FDreamMusicInformation InInformation, FDreamMusicInformationData InfomationData)
		: Information(InInformation), Data(InfomationData)
	{
	};

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformation Information;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDreamMusicInformationData Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<UDreamMusicPlayerExpansionData*> ExpansionDatas;

public:
	bool IsValid() const;
	bool operator==(const FDreamMusicDataStruct& Target) const;

	bool HasExpansionData(TSubclassOf<UDreamMusicPlayerExpansionData> ExpansionDataClass) const;

	template <typename T>
	T* GetExpansionData() const
	{
		for (auto ExpansionData : ExpansionDatas)
		{
			if (auto CastedExpansionData = Cast<T>(ExpansionData))
			{
				return CastedExpansionData;
			}
		}
		return nullptr;
	}
};

// 歌曲数据表
USTRUCT(BlueprintType)
struct FDreamMusicPlayerSongList : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDreamMusicData* MusicData;
};

USTRUCT(BlueprintType)
struct FDreamMusicPlayerFadeAudioSetting
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableFadeAudio = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutDuration = 0.5f;
};

// ============ 歌词资产相关类型定义（用于 Lyric 模块） ============

/**
 * @brief 歌词行结构
 * 
 * 表示一行歌词，可以包含多个单词（用于逐词显示）
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicLyricLine
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
	FDreamMusicLyricTimestamp GetStartTimestamp() const;

	/**
	 * @brief 获取行的结束时间（从最后一个单词）
	 */
	FDreamMusicLyricTimestamp GetEndTimestamp() const;

	/**
	 * @brief 判断时间点是否在行的时间范围内
	 */
	bool IsTimeInRange(const FDreamMusicLyricTimestamp& Time) const;

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
struct DREAMMUSICPLAYER_API FDreamMusicLyricGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FDreamMusicLyricTimestamp Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (TitleProperty = "Text"))
	TArray<FDreamMusicLyricLine> Lines;

	FDreamMusicLyricGroup() = default;

	FDreamMusicLyricGroup(const FDreamMusicLyricTimestamp& InTimestamp)
		: Timestamp(InTimestamp)
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
	bool IsTimeMatch(const FDreamMusicLyricTimestamp& Time, float ToleranceSeconds = 0.1f) const;

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
struct DREAMMUSICPLAYER_API FDreamMusicLyricMetadata
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
