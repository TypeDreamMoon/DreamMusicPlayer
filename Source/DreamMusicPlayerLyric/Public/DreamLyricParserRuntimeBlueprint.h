#pragma once

#include "CoreMinimal.h"
#include "DreamLyricTypes.h"
#include "dlp/File.hpp"
#include "DreamLyricParserRuntimeBlueprint.generated.h"

class UDreamLyricAsset;
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
	UDreamLyricAsset* Asset = nullptr;

	FDreamLyricImportResult() = default;

	FDreamLyricImportResult(bool bInSuccess, const FString& InErrorMessage, UDreamLyricAsset* InAsset = nullptr)
		: bSuccess(bInSuccess), ErrorMessage(InErrorMessage), Asset(InAsset)
	{
	}
};

/**
 * @brief 运行时歌词解析器工具类
 * 
 * 使用第三方 DreamLyricParser 库实现运行时歌词文件导入功能
 * 参考 DreamMusicPlayer/Public 中的实现方式
 */
UCLASS(BlueprintType)
class DREAMMUSICPLAYERLYRIC_API UDreamLyricParserRuntimeBlueprint : public UBlueprintFunctionLibrary
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
		EDreamMusicPlayerLyricType Format = EDreamMusicPlayerLyricType::LRC,
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
		EDreamMusicPlayerLyricType Format,
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
	static EDreamMusicPlayerLyricType DetectFileFormat(const FString& FilePath);

	/**
	 * @brief 检查文件是否可以导入
	 * 
	 * @param FilePath 文件路径
	 * @return bool 是否可以导入
	 */
	UFUNCTION(BlueprintCallable, Category = "Dream Music Player|Lyric|Runtime Import", CallInEditor)
	static bool CanImportFile(const FString& FilePath);

	/**
	 * @brief 从文件扩展名检测格式
	 */
	static EDreamMusicPlayerLyricType DetectFormatFromExtension(const FString& Extension);

	/**
	 * @brief 使用第三方库解析歌词内容
	 */
	static bool ParseLyricContent(
		const FString& FileContent,
		dlp::EFileFormat Format,
		UDreamLyricAsset* OutAsset,
		FString& OutErrorMessage,
		const FDreamLyricParserOptions& ParserOptions = FDreamLyricParserOptions()
	);

	static void ConvertParsedLyric(dlp::File::FLyricFile ParsedFile, TArray<FDreamMusicLyricGroup>& OutGroups);

	/**
	 * @brief 将第三方库的解析结果转换为 ULyricAsset
	 */
	static void ConvertParsedLyric(const dlp::File::FLyricFile& ParsedFile, UDreamLyricAsset* Asset);
};
