#include "DreamLyricAssetConverter.h"
#include "DreamLyricAsset.h"
#include "DreamMusicPlayerCommon.h"

FDreamMusicLyricTimestamp ULyricAssetConverter::ConvertTimestamp(const FDreamMusicLyricTimestamp& OldTimestamp)
{
	// 直接返回，因为现在使用相同的类型
	return OldTimestamp;
}

FDreamMusicLyricGroup ULyricAssetConverter::ConvertLyric(const FDreamMusicLyric& OldLyric)
{
	FDreamMusicLyricGroup Group(OldLyric.StartTimestamp);

	// 转换原歌词（使用 Content 字段）
	if (!OldLyric.Content.IsEmpty())
	{
		FDreamMusicLyricLine LyricLine(OldLyric.Content, EDreamMusicLyricTextRole::Lyric);
		
		// 转换单词时间戳（使用 WordTimings）
		const TArray<FDreamMusicLyricWord>& WordsToConvert = OldLyric.WordTimings;
		for (const FDreamMusicLyricWord& OldWord : WordsToConvert)
		{
			FDreamMusicLyricWord NewWord;
			NewWord.Content = OldWord.Content;
			NewWord.StartTimestamp = OldWord.StartTimestamp;
			NewWord.EndTimestamp = OldWord.EndTimestamp;
			NewWord.bHasEndTimestamp = OldWord.EndTimestamp.ToMilliseconds() > 0;
			NewWord.Role = EDreamMusicLyricTextRole::Lyric;
			LyricLine.Words.Add(NewWord);
		}
		
		Group.Lines.Add(LyricLine);
	}

	// 转换音译
	if (!OldLyric.Romanization.IsEmpty())
	{
		FDreamMusicLyricLine RomanizationLine(OldLyric.Romanization, EDreamMusicLyricTextRole::Romanization);
		
		// 转换音译单词时间戳
		for (const FDreamMusicLyricWord& OldWord : OldLyric.RomanizationWordTimings)
		{
			FDreamMusicLyricWord NewWord;
			NewWord.Content = OldWord.Content;
			NewWord.StartTimestamp = OldWord.StartTimestamp;
			NewWord.EndTimestamp = OldWord.EndTimestamp;
			NewWord.bHasEndTimestamp = OldWord.EndTimestamp.ToMilliseconds() > 0;
			NewWord.Role = EDreamMusicLyricTextRole::Romanization;
			RomanizationLine.Words.Add(NewWord);
		}
		
		Group.Lines.Add(RomanizationLine);
	}

	// 转换翻译
	if (!OldLyric.Translate.IsEmpty())
	{
		FDreamMusicLyricLine TranslationLine(OldLyric.Translate, EDreamMusicLyricTextRole::Translation);
		Group.Lines.Add(TranslationLine);
	}

	return Group;
}

FDreamMusicLyric ULyricAssetConverter::ConvertLyric(const FDreamMusicLyricGroup& NewGroup)
{
	FDreamMusicLyric OldLyric;
	OldLyric.StartTimestamp = NewGroup.Timestamp;

	// 转换行
	for (const FDreamMusicLyricLine& Line : NewGroup.Lines)
	{
		switch (Line.Role)
		{
		case EDreamMusicLyricTextRole::Lyric:
			OldLyric.Content = Line.Text;
			// 转换单词时间戳
			for (const FDreamMusicLyricWord& Word : Line.Words)
			{
				FDreamMusicLyricWord OldWord;
				OldWord.Content = Word.Content;
				OldWord.StartTimestamp = Word.StartTimestamp;
				OldWord.EndTimestamp = Word.EndTimestamp;
				OldWord.bHasEndTimestamp = Word.bHasEndTimestamp;
				OldWord.Role = Word.Role;
				OldLyric.WordTimings.Add(OldWord);
			}
			break;
		case EDreamMusicLyricTextRole::Romanization:
			OldLyric.Romanization = Line.Text;
			// 转换音译单词时间戳
			for (const FDreamMusicLyricWord& Word : Line.Words)
			{
				FDreamMusicLyricWord OldWord;
				OldWord.Content = Word.Content;
				OldWord.StartTimestamp = Word.StartTimestamp;
				OldWord.EndTimestamp = Word.EndTimestamp;
				OldWord.bHasEndTimestamp = Word.bHasEndTimestamp;
				OldWord.Role = Word.Role;
				OldLyric.RomanizationWordTimings.Add(OldWord);
			}
			break;
		case EDreamMusicLyricTextRole::Translation:
			OldLyric.Translate = Line.Text;
			break;
		default:
			break;
		}
	}

	// 计算并设置 EndTimestamp
	// 优先使用单词时间戳的最后一个结束时间
	FDreamMusicLyricTimestamp CalculatedEndTime;
	bool bHasWordTimings = false;

	// 检查原歌词的单词时间戳
	if (!OldLyric.IsWordsEmpty())
	{
		const FDreamMusicLyricWord& LastWord = OldLyric.WordTimings.Last();
		if (LastWord.bHasEndTimestamp && LastWord.EndTimestamp.ToMilliseconds() > 0)
		{
			CalculatedEndTime = LastWord.EndTimestamp;
			bHasWordTimings = true;
		}
		else if (LastWord.StartTimestamp.ToMilliseconds() > 0)
		{
			// 如果没有结束时间，使用开始时间作为结束时间
			CalculatedEndTime = LastWord.StartTimestamp;
			bHasWordTimings = true;
		}
	}
	// 如果没有原歌词时间戳，检查音译时间戳
	else if (!OldLyric.IsRomanizationWordsEmpty())
	{
		const FDreamMusicLyricWord& LastWord = OldLyric.RomanizationWordTimings.Last();
		if (LastWord.bHasEndTimestamp && LastWord.EndTimestamp.ToMilliseconds() > 0)
		{
			CalculatedEndTime = LastWord.EndTimestamp;
			bHasWordTimings = true;
		}
		else if (LastWord.StartTimestamp.ToMilliseconds() > 0)
		{
			CalculatedEndTime = LastWord.StartTimestamp;
			bHasWordTimings = true;
		}
	}

	if (bHasWordTimings)
	{
		OldLyric.EndTimestamp = CalculatedEndTime;
	}
	else
	{
		// 如果没有单词时间戳，使用行的结束时间（如果有的话）
		// 或者使用默认持续时间
		int32 DefaultDurationMs = 3000; // 默认3秒
		int32 EndMs = OldLyric.StartTimestamp.ToMilliseconds() + DefaultDurationMs;
		OldLyric.EndTimestamp = FDreamMusicLyricTimestamp(
			EndMs / 3600000,
			(EndMs / 60000) % 60,
			(EndMs / 1000) % 60,
			EndMs % 1000
		);
	}

	return OldLyric;
}

UDreamLyricAsset* ULyricAssetConverter::ConvertLyricsToAsset(const TArray<FDreamMusicLyric>& OldLyrics, UObject* Outer)
{
	if (!Outer)
	{
		Outer = GetTransientPackage();
	}

	UDreamLyricAsset* Asset = NewObject<UDreamLyricAsset>(Outer);
	if (!Asset)
	{
		return nullptr;
	}

	// 转换所有歌词
	for (const FDreamMusicLyric& OldLyric : OldLyrics)
	{
		FDreamMusicLyricGroup Group = ConvertLyric(OldLyric);
		Asset->Groups.Add(Group);
	}

	// 排序
	Asset->SortGroupsByTime();

	return Asset;
}

TArray<FDreamMusicLyric> ULyricAssetConverter::ConvertAssetToLyrics(const UDreamLyricAsset* Asset)
{
	TArray<FDreamMusicLyric> OldLyrics;

	if (!Asset)
	{
		return OldLyrics;
	}

	// 转换所有组
	for (int32 i = 0; i < Asset->Groups.Num(); i++)
	{
		const FDreamMusicLyricGroup& Group = Asset->Groups[i];
		FDreamMusicLyric OldLyric = ConvertLyric(Group);
		
		// 如果当前歌词的 EndTimestamp 无效或未设置，使用下一个组的开始时间
		if (OldLyric.EndTimestamp.ToMilliseconds() == 0 || 
			OldLyric.EndTimestamp.ToMilliseconds() <= OldLyric.StartTimestamp.ToMilliseconds())
		{
			if (i + 1 < Asset->Groups.Num())
			{
				// 使用下一个组的开始时间作为当前组的结束时间
				OldLyric.EndTimestamp = Asset->Groups[i + 1].Timestamp;
			}
			// 如果没有下一个组，EndTimestamp 已经在 ConvertLyric 中设置了默认值
		}
		
		OldLyrics.Add(OldLyric);
	}

	return OldLyrics;
}

