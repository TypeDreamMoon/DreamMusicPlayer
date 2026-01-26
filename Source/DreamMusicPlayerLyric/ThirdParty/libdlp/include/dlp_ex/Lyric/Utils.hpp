#pragma once

#include <string>
#include <vector>
#include <optional>
#include "dlp_ex/Export.hpp" // 引入上面的导出宏
#include "dlp/Types.hpp"
#include "dlp/File.hpp"

namespace dlp::Expansion::Lyric::Utils
{
    // ==========================================
    // 基础查找功能 (Playback Utils)
    // ==========================================

    /**
     * @brief 在 LyricFile 中查找指定时间点处于激活状态的 LyricGroup。
     * 使用二分查找算法 (Binary Search)，O(log N)。
     * @return 如果在该时间点没有歌词（如前奏、间奏），返回 nullptr。
     */
    DLP_EX_API [[nodiscard]] const dlp::File::FLyricGroup* FindGroupAtTime(const dlp::File::FLyricFile& in_file, dlp::timestamp in_time);

    /**
     * @brief 查找当前时间点最接近的上一句歌词。
     * 即使当前处于间奏，也会返回刚结束的那一句，适合做 "Last Played" 显示。
     */
    DLP_EX_API [[nodiscard]] const dlp::File::FLyricGroup* FindCurrentOrPrevGroup(const dlp::File::FLyricFile& in_file, dlp::timestamp in_time);

    /**
     * @brief 查找当前 Group 的索引。
     */
    DLP_EX_API [[nodiscard]] std::optional<size_t> FindGroupIndexAtTime(const dlp::File::FLyricFile& in_file, dlp::timestamp in_time);


    // ==========================================
    // 逐字进度功能 (Karaoke Utils)
    // ==========================================

    /**
     * @brief 单个单词的进度结果结构体
     */
    struct FWordProgress
    {
        size_t index = 0; // 当前单词索引
        double progress = 0.0; // 当前单词完成度 (0.0 ~ 1.0)
        bool is_completed_line = false; // 整行是否播放完毕
        const dlp::File::FLyricWord* current_word = nullptr; // 指向当前单词数据的指针
    };

    /**
     * @brief 计算当前时间点在指定歌词行中的逐字进度。
     */
    DLP_EX_API [[nodiscard]] FWordProgress GetLineWordProgress(const dlp::File::FLyricLine& in_line, dlp::timestamp in_time);

    /**
     * @brief 计算整行歌词的播放进度 (0.0 ~ 1.0)。
     * 适用于绘制整行的背景扫光效果 (Wipe effect)。
     * 无论是否是 WordByWord，都基于 Line 的总 Start/End 时间计算。
     * * @param in_line 目标歌词行
     * @param in_time 当前时间
     * @return double 进度值 (0.0 表示未开始, 1.0 表示已结束)
     */
    DLP_EX_API [[nodiscard]] double GetLineProgress(const dlp::File::FLyricLine& in_line, dlp::timestamp in_time);


    // ==========================================
    // 编辑与搜索功能 (Edit & Search Utils)
    // ==========================================

    /**
     * @brief 标准化歌词文件 (Normalize)。
     * 1. 按时间戳排序 Group。
     * 2. 移除无效空 Group。
     * 3. 修正时间重叠/倒流。
     * *注意：这会修改传入的 file 对象。*
     */
    DLP_EX_API void NormalizeLyric(dlp::File::FLyricFile& in_out_file);

    /**
     * @brief 全局时间偏移。
     */
    DLP_EX_API void ApplyGlobalOffset(dlp::File::FLyricFile& in_out_file, dlp::timestamp in_offset);

    /**
     * @brief 文本提取（扁平化）。
     */
    DLP_EX_API [[nodiscard]] std::string FlattenGroupText(const dlp::File::FLyricGroup& in_group, std::string_view in_separator = "\n");

    /**
     * @brief 搜索结果结构体
     */
    struct FSearchResult
    {
        const dlp::File::FLyricGroup* group = nullptr;
        double similarity_score = 0.0; // 0.0 ~ 1.0
        std::string matched_text;
    };

    /**
     * @brief 模糊搜索 (Fuzzy Search)。
     * 基于 Levenshtein Distance。
     * @param in_threshold 最小相似度阈值 (默认 0.5)
     */
    DLP_EX_API [[nodiscard]] std::vector<FSearchResult> SearchLyric(
        const dlp::File::FLyricFile& in_file,
        std::string_view in_query,
        double in_threshold = 0.5
    );
}
