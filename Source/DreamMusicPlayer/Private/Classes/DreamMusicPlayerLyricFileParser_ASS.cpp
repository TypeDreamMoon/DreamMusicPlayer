#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicPlayerLyricFileParser.h"

void FDreamMusicPlayerLyricFileParser_ASS::Parse()
{
	bool bIsEvent = false;

	for (const FString& SourceLine : Lines)
	{
		FString Line = SourceLine.TrimStartAndEnd();

		if (Line == TEXT("[Events]"))
		{
			bIsEvent = true;
			continue;
		}

		if (bIsEvent && Line.StartsWith(TEXT("Dialogue:"), ESearchCase::CaseSensitive))
		{
			// 跳过"Dialogue:"部分
			FString DialogueContent = Line.RightChop(9);

			// 按逗号分割字段
			TArray<FString> Parts;
			DialogueContent.ParseIntoArray(Parts, TEXT(","), false);

			if (Parts.Num() < 10)
			{
				DMP_LOG(Warning, TEXT("Invalid Dialogue Line: %s Parts: %d"), *Line, Parts.Num())
				continue;
			}

			// 获取时间信息和样式
			FString StartTimeStr = Parts[1];
			FString EndTimeStr = Parts[2];
			FString Style = Parts[3]; // Style类型: orig, ts, roma
			FString Text = Parts[9];

			// 处理可能被逗号分割的文本部分
			for (int32 i = 10; i < Parts.Num(); i++)
			{
				Text += TEXT(",") + Parts[i];
			}

			DMP_LOG(Log, TEXT("Parsing Dialogue - Style: %s, Start: %s, End: %s, Text: %s"),
			        *Style, *StartTimeStr, *EndTimeStr, *Text);

			// 解析时间戳 (ASS格式: HH:MM:SS.cc 或 HH:MM:SS.fff)
			FDreamMusicLyricTimestamp StartTimestamp = ParseASSTimestamp(StartTimeStr);
			FDreamMusicLyricTimestamp EndTimestamp = ParseASSTimestamp(EndTimeStr);

			DMP_LOG(Log, TEXT("Parsed Timestamps - Start ms: %d, End ms: %d"),
			        StartTimestamp.TotalMilliseconds(), EndTimestamp.TotalMilliseconds());

			// 根据样式类型处理
			if (Style == TEXT("orig"))
			{
				// 查找是否已存在相同时间的歌词行
				FDreamMusicLyric* ExistingLyric = nullptr;
				for (FDreamMusicLyric& Lyric : ParsedLyrics)
				{
					if (Lyric.Timestamp.TotalMilliseconds() == StartTimestamp.TotalMilliseconds())
					{
						ExistingLyric = &Lyric;
						break;
					}
				}

				if (ExistingLyric)
				{
					// 如果已存在，更新原文内容
					ExistingLyric->Content = Text;
					ExistingLyric->EndTimestamp = EndTimestamp;
				}
				else
				{
					// 创建新的歌词行
					FDreamMusicLyric Lyric;
					Lyric.Timestamp = StartTimestamp;
					Lyric.EndTimestamp = EndTimestamp;
					Lyric.Content = Text;
					ParsedLyrics.Add(Lyric);
				}
			}
			else if (Style == TEXT("ts"))
			{
				// 查找对应的原文行
				bool bFound = false;
				for (FDreamMusicLyric& Lyric : ParsedLyrics)
				{
					if (Lyric.Timestamp.TotalMilliseconds() == StartTimestamp.TotalMilliseconds())
					{
						Lyric.Translate = Text;
						bFound = true;
						break;
					}
				}

				// 如果没有找到对应的原文行，则创建新的
				if (!bFound)
				{
					FDreamMusicLyric Lyric;
					Lyric.Timestamp = StartTimestamp;
					Lyric.EndTimestamp = EndTimestamp;
					Lyric.Translate = Text;
					ParsedLyrics.Add(Lyric);
				}
			}
			else if (Style == TEXT("roma"))
			{
				// 查找对应的原文行
				bool bFound = false;
				for (FDreamMusicLyric& Lyric : ParsedLyrics)
				{
					if (Lyric.Timestamp.TotalMilliseconds() == StartTimestamp.TotalMilliseconds())
					{
						Lyric.Romanization = Text;
						// 同时处理罗马音的卡拉OK标签
						ProcessRomanizationKaraokeTags(Lyric);
						bFound = true;
						break;
					}
				}

				// 如果没有找到对应的原文行，则创建新的
				if (!bFound)
				{
					FDreamMusicLyric Lyric;
					Lyric.Timestamp = StartTimestamp;
					Lyric.EndTimestamp = EndTimestamp;
					Lyric.Romanization = Text;
					// 处理罗马音的卡拉OK标签
					ProcessRomanizationKaraokeTags(Lyric);
					ParsedLyrics.Add(Lyric);
				}
			}
		}
	}

	// 处理所有原文行的卡拉OK标签
	for (FDreamMusicLyric& Lyric : ParsedLyrics)
	{
		if (!Lyric.Content.IsEmpty())
		{
			ProcessKaraokeTags(Lyric);
		}

		DMP_LOG(Log, TEXT("Final Lyric: %s"), *Lyric.ToString());
	}
}


void FDreamMusicPlayerLyricFileParser_ASS::ProcessText(FDreamMusicLyric& Lyric)
{
	// 可以在这里处理卡拉OK时间标签等特殊格式
	// 对于原文(orig)行，可以解析{\kfXX}标签获取单词时间信息
	// 对于翻译(ts)和罗马音(roma)行，通常不需要特殊处理

	if (!Lyric.Content.IsEmpty())
	{
		// 处理原文中的卡拉OK时间标签
		ProcessKaraokeTags(Lyric);
	}
}

FDreamMusicLyricTimestamp FDreamMusicPlayerLyricFileParser_ASS::ParseASSTimestamp(const FString& TimestampStr)
{
	// ASS时间格式: HH:MM:SS.cc 或 HH:MM:SS.fff (cc是厘秒，fff是毫秒)
	// 示例: 00:00:00.17 或 00:01:13.84

	TArray<FString> TimeParts;
	TimestampStr.ParseIntoArray(TimeParts, TEXT(":"));

	if (TimeParts.Num() != 3)
	{
		return FDreamMusicLyricTimestamp();
	}

	int32 Hours = FCString::Atoi(*TimeParts[0]);
	int32 Minutes = FCString::Atoi(*TimeParts[1]);

	// 处理秒和小数部分
	FString SecondPart = TimeParts[2];
	TArray<FString> SecondParts;
	SecondPart.ParseIntoArray(SecondParts, TEXT("."));

	int32 Seconds = FCString::Atoi(*SecondParts[0]);
	int32 Milliseconds = 0;

	if (SecondParts.Num() > 1)
	{
		FString Fraction = SecondParts[1];
		if (Fraction.Len() == 2)
		{
			// 厘秒格式 (cc)，需要转换为毫秒
			Milliseconds = FCString::Atoi(*Fraction) * 10;
		}
		else if (Fraction.Len() == 3)
		{
			// 毫秒格式 (fff)
			Milliseconds = FCString::Atoi(*Fraction);
		}
		else if (Fraction.Len() == 1)
		{
			// 十分之一秒格式 (c)
			Milliseconds = FCString::Atoi(*Fraction) * 100;
		}
	}

	return FDreamMusicLyricTimestamp(Hours, Minutes, Seconds, Milliseconds);
}

void FDreamMusicPlayerLyricFileParser_ASS::ProcessKaraokeTags(FDreamMusicLyric& Lyric)
{
	// 解析原文中的卡拉OK时间标签，例如: {\kf16}ウ{\kf19}タ{\kf8}オ
	FString Text = Lyric.Content;

	// 使用正则表达式查找{\kfXX}标签
	FRegexPattern Pattern(TEXT("\\{\\\\kf(\\d+)\\}([^\\{]*)"));
	FRegexMatcher Matcher(Pattern, Text);

	int32 CurrentTimeMs = Lyric.Timestamp.TotalMilliseconds();

	// 清除已存在的单词时间信息
	Lyric.WordTimings.Empty();

	FString CleanText = TEXT(""); // 构建清理后的文本

	while (Matcher.FindNext())
	{
		FString DurationStr = Matcher.GetCaptureGroup(1);
		FString Character = Matcher.GetCaptureGroup(2);

		// 处理可能的空字符情况
		if (Character.IsEmpty())
			continue;

		int32 Duration = FCString::Atoi(*DurationStr) * 10; // 转换为毫秒

		FDreamMusicLyricTimestamp StartTs, EndTs;
		// 根据毫秒数计算出时分秒
		StartTs.Millisecond = CurrentTimeMs % 1000;
		StartTs.Seconds = (CurrentTimeMs / 1000) % 60;
		StartTs.Minute = (CurrentTimeMs / 60000) % 60;
		StartTs.Hours = CurrentTimeMs / 3600000;

		int32 EndTimeMs = CurrentTimeMs + Duration;
		EndTs.Millisecond = EndTimeMs % 1000;
		EndTs.Seconds = (EndTimeMs / 1000) % 60;
		EndTs.Minute = (EndTimeMs / 60000) % 60;
		EndTs.Hours = EndTimeMs / 3600000;

		FDreamMusicLyricWord Word(StartTs, EndTs, Character);
		Lyric.WordTimings.Add(Word);

		// 构建清理后的文本
		CleanText += Character;

		CurrentTimeMs = EndTimeMs;
	}

	// 如果解析到了带标签的内容，使用清理后的文本
	if (Lyric.WordTimings.Num() > 0)
	{
		Lyric.Content = CleanText;
	}
	else
	{
		// 如果没有找到标签，尝试清理可能残留的标签
		FString CleanedText = Text;
		CleanedText = CleanedText.Replace(TEXT("{\\kf"), TEXT(""));
		CleanedText = CleanedText.Replace(TEXT("{"), TEXT(""));
		CleanedText = CleanedText.Replace(TEXT("}"), TEXT(""));
		Lyric.Content = CleanedText;
	}
}

void FDreamMusicPlayerLyricFileParser_ASS::ProcessRomanizationKaraokeTags(FDreamMusicLyric& Lyric)
{
	// 解析罗马音中的卡拉OK时间标签，例如: {\kf16}u {\kf19}ta {\kf8}o
	FString Text = Lyric.Romanization;

	// 使用正则表达式查找{\kfXX}标签
	FRegexPattern Pattern(TEXT("\\{\\\\kf(\\d+)\\}([^\\{]*)"));
	FRegexMatcher Matcher(Pattern, Text);

	int32 CurrentTimeMs = Lyric.Timestamp.TotalMilliseconds();

	// 清除已存在的罗马音单词时间信息
	Lyric.RomanizationWordTimings.Empty();

	FString CleanText = TEXT(""); // 构建清理后的文本

	while (Matcher.FindNext())
	{
		FString DurationStr = Matcher.GetCaptureGroup(1);
		FString Character = Matcher.GetCaptureGroup(2);

		// 处理可能的空字符情况
		if (Character.IsEmpty())
			continue;

		int32 Duration = FCString::Atoi(*DurationStr) * 10; // 转换为毫秒

		FDreamMusicLyricTimestamp StartTs, EndTs;
		// 根据毫秒数计算出时分秒
		StartTs.Millisecond = CurrentTimeMs % 1000;
		StartTs.Seconds = (CurrentTimeMs / 1000) % 60;
		StartTs.Minute = (CurrentTimeMs / 60000) % 60;
		StartTs.Hours = CurrentTimeMs / 3600000;

		int32 EndTimeMs = CurrentTimeMs + Duration;
		EndTs.Millisecond = EndTimeMs % 1000;
		EndTs.Seconds = (EndTimeMs / 1000) % 60;
		EndTs.Minute = (EndTimeMs / 60000) % 60;
		EndTs.Hours = EndTimeMs / 3600000;

		FDreamMusicLyricWord Word(StartTs, EndTs, Character);
		Lyric.RomanizationWordTimings.Add(Word);

		// 构建清理后的文本
		CleanText += Character;

		CurrentTimeMs = EndTimeMs;
	}

	// 如果解析到了带标签的内容，使用清理后的文本
	if (Lyric.RomanizationWordTimings.Num() > 0)
	{
		Lyric.Romanization = CleanText;
	}
	else
	{
		// 如果没有找到标签，尝试清理可能残留的标签
		FString CleanedText = Text;
		CleanedText = CleanedText.Replace(TEXT("{\\kf"), TEXT(""));
		CleanedText = CleanedText.Replace(TEXT("{"), TEXT(""));
		CleanedText = CleanedText.Replace(TEXT("}"), TEXT(""));
		Lyric.Romanization = CleanedText;
	}
}
