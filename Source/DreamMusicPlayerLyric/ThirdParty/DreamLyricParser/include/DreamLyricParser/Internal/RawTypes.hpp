#pragma once
#include <string>
#include <vector>
#include "DreamLyricParser/Types.hpp"

namespace dream_lyric_parser {

    // 原始歌词行：这是解析器（LRC/YRC/TTML）吐出的最基础单位
    // 此时我们还不知道它到底是翻译、罗马音还是主歌词，只知道它在某个时间点有一段文本
    struct FRawLyricLine {
        int64_t StartTimeMs;       // 开始时间 (毫秒)
        int64_t DurationMs = 0;    // 持续时间 (0 表示未知，通常 LRC 解析出来都是 0)
        std::string Content;       // 文本内容

        // 这是一个启发式标记，解析器如果看到括号 ( ) 可能会把这个设为 true
        bool bIsPotentialBackground{false};

        // 用于排序：按时间先后排序
        bool operator<(const FRawLyricLine& Other) const {
            return StartTimeMs < Other.StartTimeMs;
        }
    };

}