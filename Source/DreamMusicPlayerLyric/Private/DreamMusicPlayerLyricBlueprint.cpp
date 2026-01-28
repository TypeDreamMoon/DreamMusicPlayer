// Fill out your copyright notice in the Description page of Project Settings.


#include "DreamMusicPlayerLyricBlueprint.h"

#include "DreamLyricUtils.h"

// ==========================================
// 内部辅助函数
// ==========================================
namespace DreamLyricUtils
{
	/**
	 * Levenshtein Distance 算法的 FString 版本
	 */
	int32 ComputeLevenshteinDistance(const FString& Source, const FString& Target)
	{
		const int32 Len1 = Source.Len();
		const int32 Len2 = Target.Len();

		if (Len1 == 0) return Len2;
		if (Len2 == 0) return Len1;

		// 使用 TArray 替代 std::vector
		TArray<int32> Costs;
		Costs.SetNum(Len2 + 1);

		// 初始化第一行: 0, 1, 2, ...
		for (int32 j = 0; j <= Len2; ++j)
		{
			Costs[j] = j;
		}

		for (int32 i = 0; i < Len1; ++i)
		{
			int32 Prev = Costs[0];
			Costs[0] = i + 1;

			for (int32 j = 0; j < Len2; ++j)
			{
				int32 Temp = Costs[j + 1];

				// 计算替换、插入、删除的代价
				const int32 Cost = (Source[i] == Target[j]) ? 0 : 1;

				Costs[j + 1] = FMath::Min3(
					Prev + Cost, // 替换/相等
					Costs[j] + 1, // 插入
					Costs[j + 1] + 1 // 删除
				);

				Prev = Temp;
			}
		}

		return Costs[Len2];
	}

	/**
	 * 辅助计算两个 Timestamp 之间的秒数差 (double)
	 */
	double GetDurationSeconds(const FDreamMusicTimestamp& Start, const FDreamMusicTimestamp& End)
	{
		// 假设 FDreamMusicTimestamp 有 ToSeconds() 方法，或者通过 Milliseconds 转换
		// 根据你的头文件，这里使用 ToTotalMilliseconds 更安全，精度更高
		int64 StartMs = Start.ToTotalMilliseconds();
		int64 EndMs = End.ToTotalMilliseconds();

		return (double)(EndMs - StartMs) / 1000.0;
	}
}

// ==========================================
// 蓝图函数库实现
// ==========================================

TArray<FString> UDreamMusicPlayerLyricBlueprint::GetLyricFileNames()
{
	return FDreamLyricUtils::GetLyricFileNames();
}

FDreamMusicLyricGroup UDreamMusicPlayerLyricBlueprint::FindGroupAtTime(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp)
{
	if (Groups.IsEmpty())
	{
		return FDreamMusicLyricGroup();
	}

	// 使用 Algo::LowerBound 实现二分查找
	// LowerBound 返回第一个 !Predicate(Element) 的位置，即第一个 StartTime >= Timestamp 的元素
	int32 Index = Algo::LowerBound(Groups, Timestamp, [](const FDreamMusicLyricGroup& Group, const FDreamMusicTimestamp& Time)
	{
		return Group.StartTimestamp < Time;
	});

	// Index 现在指向第一个 StartTime >= Timestamp 的 Group

	// Case 1: 检查是否正好落在该 Index 组内 (特别是当 StartTime == Timestamp 时)
	if (Index < Groups.Num())
	{
		const FDreamMusicLyricGroup& Candidate = Groups[Index];
		if (Timestamp >= Candidate.StartTimestamp && Timestamp < Candidate.EndTimestamp)
		{
			return Candidate;
		}
	}

	// Case 2: 检查前一个组 (通常情况，因为 LowerBound 找到的是"未来"或"现在刚开始"的组)
	// 如果 Timestamp 是 00:05，而 Index 指向 00:10，我们需要看 Index-1 (00:04-00:08)
	if (Index > 0)
	{
		const FDreamMusicLyricGroup& Prev = Groups[Index - 1];
		if (Timestamp >= Prev.StartTimestamp && Timestamp < Prev.EndTimestamp)
		{
			return Prev;
		}
	}

	// 未找到激活的组 (可能在间奏)
	return FDreamMusicLyricGroup();
}

int32 UDreamMusicPlayerLyricBlueprint::FindGroupIndexAtTime(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp)
{
	if (Groups.IsEmpty())
	{
		return -1;
	}

	// 逻辑同上，只是返回索引
	int32 Index = Algo::LowerBound(Groups, Timestamp, [](const FDreamMusicLyricGroup& Group, const FDreamMusicTimestamp& Time)
	{
		return Group.StartTimestamp < Time;
	});

	if (Index < Groups.Num())
	{
		if (Timestamp >= Groups[Index].StartTimestamp && Timestamp < Groups[Index].EndTimestamp)
		{
			return Index;
		}
	}

	if (Index > 0)
	{
		const FDreamMusicLyricGroup& Prev = Groups[Index - 1];
		if (Timestamp >= Prev.StartTimestamp && Timestamp < Prev.EndTimestamp)
		{
			return Index - 1;
		}
	}

	return -1;
}

FDreamMusicLyricGroup UDreamMusicPlayerLyricBlueprint::FindCurrentOrPrevGroup(const TArray<FDreamMusicLyricGroup>& Groups, const FDreamMusicTimestamp& Timestamp)
{
	if (Groups.IsEmpty())
	{
		return FDreamMusicLyricGroup();
	}

	// 使用 Algo::UpperBound 查找第一个 StartTime > Timestamp 的位置
	int32 Index = Algo::UpperBound(Groups, Timestamp, [](const FDreamMusicTimestamp& Time, const FDreamMusicLyricGroup& Group)
	{
		return Time < Group.StartTimestamp;
	});

	// 如果 Index 为 0，说明所有组都在未来，当前还没开始
	if (Index == 0)
	{
		return FDreamMusicLyricGroup();
	}

	// 返回前一个组 (即最后一个 StartTime <= Timestamp 的组)
	// 即使这句歌词已经结束了（进入间奏），我们依然返回它作为“最近一句”
	return Groups[Index - 1];
}

FDreamMusicLyricProgress UDreamMusicPlayerLyricBlueprint::GetLineWordProgress(const FDreamMusicLyricLine& Line, const FDreamMusicTimestamp& Timestamp, bool bUseWordInterpolation)
{
	FDreamMusicLyricProgress Result;
    Result.LineProgress = 0.0f;
    Result.CurrentWordIndex = -1;
    Result.bIsActive = false;

    // 基础数据获取
    FDreamMusicTimestamp LineStart = Line.GetStartTimestamp();
    FDreamMusicTimestamp LineEnd = Line.GetEndTimestamp();
    
    // ---------------------------------------------------------
    // 0. 通用状态检查：整行是否处于 Active 状态
    // ---------------------------------------------------------
    if (Timestamp >= LineStart && Timestamp <= LineEnd)
    {
        Result.bIsActive = true;
    }
    else
    {
        Result.bIsActive = false;
        // 如果时间还没到，直接返回空进度
        if (Timestamp < LineStart) 
        {
            if (!Line.Words.IsEmpty()) 
            {
                Result.CurrentWordIndex = 0;
                Result.CurrentWord = Line.Words[0];
            }
            return Result; 
        }
        // 如果时间已过，返回满进度
        if (Timestamp > LineEnd) 
        {
            Result.LineProgress = 1.0f;
            if (!Line.Words.IsEmpty()) 
            {
                Result.CurrentWordIndex = Line.Words.Num() - 1;
                Result.CurrentWord = Line.Words.Last();
            }
            return Result;
        }
    }

    // ---------------------------------------------------------
    // 1. 模式 A: 整行进度模式 (Whole Line Mode)
    // ---------------------------------------------------------
    if (!bUseWordInterpolation)
    {
        double TotalDuration = DreamLyricUtils::GetDurationSeconds(LineStart, LineEnd);
        if (TotalDuration > 0.0)
        {
            double Elapsed = DreamLyricUtils::GetDurationSeconds(LineStart, Timestamp);
            Result.LineProgress = FMath::Clamp((float)(Elapsed / TotalDuration), 0.0f, 1.0f);
        }
        else
        {
            Result.LineProgress = 1.0f;
        }

        // 即使是整行模式，为了方便UI高亮，我们依然尝试计算当前处于哪个字
        // 如果不需要这个功能，可以把下面这段删掉
        if (!Line.Words.IsEmpty())
        {
            for (int32 i = 0; i < Line.Words.Num(); ++i)
            {
                // 如果时间小于该词结束时间，就是这个词（包含Gap期也会算在前一个词或后一个词，这里简单判定）
                if (Timestamp < Line.Words[i].EndTimestamp)
                {
                    Result.CurrentWordIndex = i;
                    Result.CurrentWord = Line.Words[i];
                    break;
                }
            }
            // 如果遍历完都没找到（比如在最后一个字的Gap里），设为最后一个
            if (Result.CurrentWordIndex == -1)
            {
                Result.CurrentWordIndex = Line.Words.Num() - 1;
                Result.CurrentWord = Line.Words.Last();
            }
        }
        
        return Result;
    }

    // ---------------------------------------------------------
    // 2. 模式 B: 逐字进度模式 (Per Word / Karaoke Mode)
    // ---------------------------------------------------------
    
    // 如果没有单词，退化回整行逻辑或直接返回
    if (Line.Words.IsEmpty())
    {
        return Result; // Progress 0, Active based on check above
    }

    const int32 Count = Line.Words.Num();

    // 2.1 边界检查：早于第一个字
    if (Timestamp < Line.Words[0].StartTimestamp)
    {
        Result.CurrentWordIndex = 0;
        Result.CurrentWord = Line.Words[0];
        Result.LineProgress = 0.0f; 
        return Result;
    }

    // 2.2 边界检查：晚于最后一个字
    if (Timestamp >= Line.Words.Last().EndTimestamp)
    {
        Result.CurrentWordIndex = Count - 1;
        Result.CurrentWord = Line.Words.Last();
        Result.LineProgress = 1.0f;
        return Result;
    }

    // 2.3 遍历查找
    for (int32 i = 0; i < Count; ++i)
    {
        const FDreamMusicLyricWord& Word = Line.Words[i];

        // Case: 时间落在当前单词内部
        if (Timestamp >= Word.StartTimestamp && Timestamp < Word.EndTimestamp)
        {
            Result.CurrentWordIndex = i;
            Result.CurrentWord = Word;
            Result.bIsActive = true;

            double WordDuration = DreamLyricUtils::GetDurationSeconds(Word.StartTimestamp, Word.EndTimestamp);
            if (WordDuration > KINDA_SMALL_NUMBER) // 也就是 > 0
            {
                double Elapsed = DreamLyricUtils::GetDurationSeconds(Word.StartTimestamp, Timestamp);
                Result.LineProgress = FMath::Clamp((float)(Elapsed / WordDuration), 0.0f, 1.0f);
            }
            else
            {
                Result.LineProgress = 1.0f;
            }
            return Result;
        }

        // Case: Gap 期 (当前字结束了，下一个字还没开始)
        // 逻辑：光标应该停留在"下一个字"的起始位置等待，进度为0
        if (i + 1 < Count)
        {
            const FDreamMusicLyricWord& NextWord = Line.Words[i + 1];
            if (Timestamp >= Word.EndTimestamp && Timestamp < NextWord.StartTimestamp)
            {
                Result.CurrentWordIndex = i + 1;
                Result.CurrentWord = NextWord;
                Result.LineProgress = 0.0f; // 下一个字还没唱，进度0
                Result.bIsActive = true;    // 依然算行活跃
                return Result;
            }
        }
    }

    return Result;
}

FDreamMusicLyricProgress UDreamMusicPlayerLyricBlueprint::GetLineProgress(const FDreamMusicLyricLine& Line, const FDreamMusicTimestamp& Timestamp)
{
	FDreamMusicLyricProgress Result;
	Result.CurrentWordIndex = -1; // 此函数不关注单词索引

	// 获取行的时间范围
	FDreamMusicTimestamp LineStart = Line.GetStartTimestamp();
	FDreamMusicTimestamp LineEnd = Line.GetEndTimestamp();

	// 1. 还没开始
	if (Timestamp < LineStart)
	{
		Result.LineProgress = 0.0f;
		Result.bIsActive = false;
		return Result;
	}

	// 2. 已经结束
	if (Timestamp >= LineEnd)
	{
		Result.LineProgress = 1.0f;
		Result.bIsActive = false;
		return Result;
	}

	// 3. 计算中间进度
	double Duration = DreamLyricUtils::GetDurationSeconds(LineStart, LineEnd);

	// 防御性编程
	if (Duration <= 0.0)
	{
		Result.LineProgress = 1.0f;
		Result.bIsActive = false;
		return Result;
	}

	double Elapsed = DreamLyricUtils::GetDurationSeconds(LineStart, Timestamp);
	Result.LineProgress = (float)(Elapsed / Duration);
	Result.bIsActive = true;

	return Result;
}

TArray<FDreamMusicLyricSearchResult> UDreamMusicPlayerLyricBlueprint::SearchLyric(const TArray<FDreamMusicLyricGroup>& Groups, const FString& Query, double Threshold)
{
	TArray<FDreamMusicLyricSearchResult> Results;

	if (Query.IsEmpty() || Groups.IsEmpty())
	{
		return Results;
	}

	for (const FDreamMusicLyricGroup& Group : Groups)
	{
		// 遍历组内的所有行 (Line)
		for (const FDreamMusicLyricLine& Line : Group.Lines)
		{
			// 计算编辑距离
			int32 Distance = DreamLyricUtils::ComputeLevenshteinDistance(Query, Line.Text);

			// 计算相似度 (1 - distance / max_length)
			int32 MaxLen = FMath::Max(Query.Len(), Line.Text.Len());
			double Similarity = (MaxLen > 0) ? (1.0 - (double)Distance / (double)MaxLen) : 0.0;

			if (Similarity >= Threshold)
			{
				FDreamMusicLyricSearchResult SearchRes;
				SearchRes.Group = Group;
				SearchRes.SimilarityScore = Similarity;
				SearchRes.MatchedText = Line.Text;

				Results.Add(SearchRes);

				// 优化：每个 Group 只要有一个匹配就行，避免同一个 Group 的原版和翻译都加进去
				// 如果需要都加，可以注释掉这个 break
				break;
			}
		}
	}

	// 按相似度降序排序
	Algo::Sort(Results, [](const FDreamMusicLyricSearchResult& A, const FDreamMusicLyricSearchResult& B)
	{
		return A.SimilarityScore > B.SimilarityScore;
	});

	return Results;
}

FDreamMusicLyricLine UDreamMusicPlayerLyricBlueprint::FindLine(const FDreamMusicLyricGroup& Group, EDreamMusicLyricTextRole Role)
{
	if (auto* ptr = Group[Role])
	{
		return *ptr;
	}
	else
	{
		return FDreamMusicLyricLine();
	}
}

bool UDreamMusicPlayerLyricBlueprint::HasLine(const FDreamMusicLyricGroup& Group, EDreamMusicLyricTextRole Role)
{
	return Group[Role] != nullptr;
}
