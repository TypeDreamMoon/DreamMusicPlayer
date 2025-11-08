#pragma once

#include "CoreMinimal.h"
#include "LyricAsset.h"
#include "DreamLyricParser/DreamLyricParser.hpp"
#include "DreamLyricParser/Types.hpp"
#include "DreamLyricParserRuntime.generated.h"

/**
 * @brief 歌词解析格式枚举（用于运行时导入）
 */
UENUM(BlueprintType)
enum class EDreamLyricParserFormat : uint8
{
	LrcLineByLine = 0,
	LrcWordByWord = 1,
	LrcEsLyric = 2,
	Srt = 3,
	Ass = 4
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
	TArray<ELyricTextRole> GroupingSequence;

	/**
	 * @brief 回退角色 - 当无法匹配序列中的角色时使用的默认角色
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parser Options")
	ELyricTextRole FallbackRole = ELyricTextRole::Lyric;

	/**
	 * @brief 使用默认配置
	 */
	static FDreamLyricParserOptions GetDefault()
	{
		FDreamLyricParserOptions Options;
		Options.GroupingSequence.Add(ELyricTextRole::Lyric);
		Options.FallbackRole = ELyricTextRole::Lyric;
		return Options;
	}

	FDreamLyricParserOptions()
	{
		GroupingSequence.Add(ELyricTextRole::Lyric);
		FallbackRole = ELyricTextRole::Lyric;
	}
};

/**
 * @brief 运行时歌词导入结果
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERLYRIC_API FDreamLyricImportResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly)
	ULyricAsset* Asset = nullptr;

	FDreamLyricImportResult() = default;
	FDreamLyricImportResult(bool bInSuccess, const FString& InErrorMessage, ULyricAsset* InAsset = nullptr)
		: bSuccess(bInSuccess), ErrorMessage(InErrorMessage), Asset(InAsset) {}
};

/**
 * @brief 运行时歌词解析器工具类
 * 
 * 使用第三方 DreamLyricParser 库实现运行时歌词文件导入功能
 * 参考 DreamMusicPlayer/Public 中的实现方式
 */
UCLASS(BlueprintType)
class DREAMMUSICPLAYERLYRIC_API UDreamLyricParserRuntime : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief 从文件路径导入歌词文件（运行时）
	 * 
	 * @param FilePath 歌词文件路径（绝对路径或相对于项目目录的路径）
	 * @param Format 解析格式（如果为 None，将自动检测）
	 * @param ParserOptions 解析选项配置（可选，如果为空则使用默认配置）
	 * @return FDreamLyricImportResult 导入结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor, meta = (AutoCreateRefTerm = "ParserOptions"))
	static FDreamLyricImportResult ImportLyricFileFromPath(
		const FString& FilePath,
		EDreamLyricParserFormat Format = EDreamLyricParserFormat::LrcLineByLine,
		const FDreamLyricParserOptions& ParserOptions = FDreamLyricParserOptions()
	);

	/**
	 * @brief 从文件内容字符串导入歌词（运行时）
	 * 
	 * @param FileContent 歌词文件内容
	 * @param Format 解析格式
	 * @param SourceFileName 源文件名（用于元数据）
	 * @param ParserOptions 解析选项配置（可选，如果为空则使用默认配置）
	 * @return FDreamLyricImportResult 导入结果
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor, meta = (AutoCreateRefTerm = "ParserOptions"))
	static FDreamLyricImportResult ImportLyricFileFromString(
		const FString& FileContent,
		EDreamLyricParserFormat Format,
		const FString& SourceFileName = TEXT(""),
		const FDreamLyricParserOptions& ParserOptions = FDreamLyricParserOptions()
	);

	/**
	 * @brief 自动检测文件格式
	 * 
	 * @param FilePath 文件路径
	 * @return EDreamLyricParserFormat 检测到的格式
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor)
	static EDreamLyricParserFormat DetectFileFormat(const FString& FilePath);

	/**
	 * @brief 检查文件是否可以导入
	 * 
	 * @param FilePath 文件路径
	 * @return bool 是否可以导入
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor)
	static bool CanImportFile(const FString& FilePath);

	/**
	 * @brief 验证第三方库 DLL 是否可用
	 * 
	 * @param OutErrorMessage 错误信息输出
	 * @return bool 是否可用
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor)
	static bool ValidateThirdPartyLibrary(FString& OutErrorMessage);
	
	/**
	 * @brief 将 UE 格式枚举转换为第三方库格式枚举
	 */
	static dream_lyric_parser::FParserFormat ConvertFormat(EDreamLyricParserFormat Format);

	/**
	 * @brief 从文件扩展名检测格式
	 */
	static EDreamLyricParserFormat DetectFormatFromExtension(const FString& Extension);

	/**
	 * @brief 使用第三方库解析歌词内容
	 */
	static bool ParseLyricContent(
		const FString& FileContent,
		dream_lyric_parser::FParserFormat Format,
		ULyricAsset* OutAsset,
		FString& OutErrorMessage,
		const FDreamLyricParserOptions& ParserOptions = FDreamLyricParserOptions()
	);

	/**
	 * @brief 将 UE 解析选项转换为第三方库解析选项
	 */
	static dream_lyric_parser::FParserOptions ConvertParserOptions(const FDreamLyricParserOptions& UEOptions);

	/**
	 * @brief 将第三方库的解析结果转换为 ULyricAsset
	 */
	static void ConvertParsedLyricToAsset(
		const dream_lyric_parser::FParsedLyric& ParsedLyric,
		ULyricAsset* Asset
	);
};

