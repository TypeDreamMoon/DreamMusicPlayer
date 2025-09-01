#pragma once

#include "DreamMusicPlayerCommon.h"

/**
 * @brief 歌词文件解析器基类
 * 
 * 该类为歌词文件解析器的抽象基类，提供歌词解析的基础框架和通用功能。
 * 子类需要实现具体的解析逻辑来处理不同格式的歌词文件。
 */
struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParserBase
{
public:
	/**
	 * @brief 禁用默认构造函数
	 * 
	 * 防止创建未初始化的解析器实例
	 */
	FDreamMusicPlayerLyricFileParserBase() = delete;

	/**
	 * @brief 构造函数
	 * 
	 * 使用指定的文件内容、行数据和行类型初始化解析器
	 * 
	 * @param InFileContent 完整的歌词文件内容字符串
	 * @param InLines 按行分割的歌词文件内容数组
	 * @param InLineType 歌词行的解析类型枚举值
	 */
	FDreamMusicPlayerLyricFileParserBase(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FileContent(InFileContent), Lines(InLines), LineType(InLineType)
	{
	}

	/**
	 * @brief 虚析构函数
	 * 
	 * 确保派生类能够正确析构
	 */
	virtual ~FDreamMusicPlayerLyricFileParserBase() = default;

protected:
	// 完整的歌词文件内容
	FString FileContent;

	// 按行分割的歌词内容数组
	TArray<FString> Lines;

	// 解析后的歌词数据数组
	TArray<FDreamMusicLyric> ParsedLyrics;

	// 歌词行解析类型
	EDreamMusicPlayerLyricParseLineType LineType;

public:
	/**
	 * @brief 执行歌词解析的纯虚函数
	 * 
	 * 子类必须实现此函数来提供具体的歌词解析逻辑
	 */
	virtual void Parse();

	/**
	 * @brief 处理歌词文本的纯虚函数
	 * 
	 * 子类必须实现此函数来处理单条歌词的文本内容
	 * 
	 * @param Lyric 需要处理的歌词对象引用
	 */
	virtual void ProcessText(FDreamMusicLyric& Lyric);

	/**
	 * @brief 获取解析后的歌词数据
	 * 
	 * @return const TArray<FDreamMusicLyric>& 解析后的歌词数组的常量引用
	 */
	const TArray<FDreamMusicLyric>& GetParsedLyrics() const { return ParsedLyrics; }

	/**
	 * @brief 清空已解析的数据
	 * 
	 * 重置解析结果，清空已存储的歌词数据
	 */
	virtual void ClearParsedData() { ParsedLyrics.Empty(); }
};


/**
 * [Lyric File Parser] SRT File Parser
 * - File format: .SRT
 */
struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_SRT : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_SRT(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType)
	{
	}

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;

protected:
	// Parse SRT timestamp line (e.g., "00:00:48,710 --> 00:00:58,770")
	bool ParseSRTTimestamp(const FString& TimestampLine, FDreamMusicLyric& OutLyric);

	// Parse individual SRT time format (HH:MM:SS,mmm)
	FDreamMusicLyricTimestamp ParseSRTTime(const FString& TimeString);

	// // Clean SRT content from formatting tags
	// FString CleanSRTContent(const FString& RawContent);
};

/**
 * [Lyric File Parser] LRC File Parser
 * - File format: .LRC
 */
struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_LRC : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_LRC(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLrcLyricType InParseMethod, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType), ParseMethod(InParseMethod)
	{
	}

protected:
	EDreamMusicPlayerLrcLyricType ParseMethod;

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;

protected:
	void ParseLinesAsTriple(const TArray<FString>& InLines, int32 LyricIndex);
	void ParseLinesAsPair(const TArray<FString>& InLines, int32 LyricIndex);
	void ParseLinesAsSingle(const TArray<FString>& InLines);

	FDreamMusicLyric CreateLyricFromLine(const FString& Line);
	FString ExtractContentFromLine(const FString& Line);
	void ProcessLyricContent(FDreamMusicLyric& Lyric, const FString& Line);
	void ProcessWordByWordContent(FDreamMusicLyric& Lyric, const FString& Line);
	void ProcessESLyricContent(FDreamMusicLyric& Lyric, const FString& Line);

	void ProcessTimestampGroup(const TArray<FString>& LinesInGroup);

	void AssignContentByType_RLT(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Romanization_Lyric_Translation
	void AssignContentByType_LRT(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Lyric_Romanization_Translation
	void AssignContentByType_TLR(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Translation_Lyric_Romanization
	void AssignContentByType_RL(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Romanization_Lyric
	void AssignContentByType_LR(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Lyric_Romanization
	void AssignContentByType_TL(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Translation_Lyric
	void AssignContentByType_LT(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Lyric_Translation
	void AssignContentByType_LO(FDreamMusicLyric& Lyric, const TArray<FString>& Lines); // Lyric_Only

	void ProcessESLyricContentForSpecificField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);
	void ProcessWordByWordContentForSpecificField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);

	FString ExtractTimestampFromLine(const FString& Line);
	int32 GetMainLyricIndex() const;

	bool IsMetadataLine(const FString& Line) const;
};

/**
 * [Lyric File Parser] ASS File Parser
 * - File format: .ASS
 */
struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_ASS : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_ASS(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType)
	{
	}

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;

protected:
	FDreamMusicLyricTimestamp ParseASSTimestamp(const FString& TimestampStr);
	void ProcessKaraokeTags(FDreamMusicLyric& Lyric);
	void ProcessRomanizationKaraokeTags(FDreamMusicLyric& Lyric);
};
