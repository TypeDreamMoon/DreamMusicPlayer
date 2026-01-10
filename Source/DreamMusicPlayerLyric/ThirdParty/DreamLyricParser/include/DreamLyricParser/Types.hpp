// DreamLyricParser - Common lyric data structures
// SPDX-License-Identifier: MIT

#pragma once

// Suppress C4251 warning: STL containers in exported classes are expected
// This is safe because we ensure proper memory management with explicit destructors/copy logic in the implementation (.cpp)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "DreamLyricParser/Export.hpp"

namespace dream_lyric_parser
{
    // 使用 typedef 方便未来可能的字符串类型替换 (如 std::u8string)
    typedef std::string FLyricString;

    namespace parser
    {
        // 歌词文件格式枚举
        enum class EParserFileFormat : int
        {
            /** Standard LRC Format [mm:ss.xx] */
            LRC = 0,
            /** ESLyric Word-by-word Format */
            ESLyric = 1,
            /** NetEase Cloud Music JSON/YRC Format */
            YRC = 2,
            /** QQ Music QRC XML Format */
            QRC = 3,
            /** Lyricify Syllable Format */
            LYS = 4,
            /** Apple Music TTML XML Format */
            // TTML = 5,
            /** Advanced Substation Alpha Subtitle */
            // ASS = 5,
            /** Dream Music Player Unified Lyric */
            // DMPUL = 7
        };

        // LRC 特定的解析选项
        enum class EParserLrcFormat : int
        {
            None = 0,
            LineByLine = 1, // 仅行
            WordByWord = 2, // 尝试解析逐字 (ESLyric)
        };
    }

    // =========================================================
    // 基础时间结构
    // =========================================================

    struct DREAMLYRICPARSER_API FTimeSpan
    {
        int hours{0};
        int minutes{0};
        int seconds{0};
        int milliseconds{0};

        constexpr FTimeSpan() = default;
        FTimeSpan(int h, int m, int s, int ms) noexcept;

        // 转换与解析
        [[nodiscard]] int64_t ToTotalMilliseconds() const noexcept;
        [[nodiscard]] static FTimeSpan FromTotalMilliseconds(int64_t total_ms) noexcept;
        [[nodiscard]] static FTimeSpan Parse(const std::string& text);
        [[nodiscard]] std::string ToString(bool include_hours = false, int fractional_digits = 3) const;

        // 归一化 (例如将 61秒 转换为 1分 1秒)
        FTimeSpan& Normalize() noexcept;
        [[nodiscard]] FTimeSpan Normalized() const noexcept;

        // 运算符重载 (保持 C++17 兼容，不使用 <=>)
        [[nodiscard]] FTimeSpan operator+(const FTimeSpan& other) const noexcept;
        [[nodiscard]] FTimeSpan operator-(const FTimeSpan& other) const noexcept;
        FTimeSpan& operator+=(const FTimeSpan& other) noexcept;
        FTimeSpan& operator-=(const FTimeSpan& other) noexcept;

        [[nodiscard]] bool operator==(const FTimeSpan& other) const noexcept;
        [[nodiscard]] bool operator!=(const FTimeSpan& other) const noexcept { return !(*this == other); }
        [[nodiscard]] bool operator<(const FTimeSpan& other) const noexcept;
        [[nodiscard]] bool operator<=(const FTimeSpan& other) const noexcept { return *this < other || *this == other; }
        [[nodiscard]] bool operator>(const FTimeSpan& other) const noexcept { return other < *this; }
        [[nodiscard]] bool operator>=(const FTimeSpan& other) const noexcept { return other <= *this; }
    };

    // =========================================================
    // 歌词角色与标记
    // =========================================================

    // 定义单行歌词的角色 (用于多语言并列显示)
    enum class FLyricTextRole : std::uint8_t
    {
        None = 0,
        Lyric = 1 << 0, // 原文
        Romanization = 1 << 1, // 罗马音
        Translation = 1 << 2 // 翻译
    };

    using FLyricTextFlags = std::uint8_t;

    // 辅助函数：将 Enum 转为 Flags
    [[nodiscard]] constexpr FLyricTextFlags ToFlags(FLyricTextRole role) noexcept
    {
        return static_cast<FLyricTextFlags>(role);
    }

    // 位运算重载 (允许 FLyricTextRole::Lyric | FLyricTextRole::Translation)
    [[nodiscard]] constexpr FLyricTextFlags operator|(FLyricTextRole lhs, FLyricTextRole rhs) noexcept
    {
        return static_cast<FLyricTextFlags>(static_cast<FLyricTextFlags>(lhs) | static_cast<FLyricTextFlags>(rhs));
    }

    [[nodiscard]] constexpr FLyricTextFlags operator|(FLyricTextFlags lhs, FLyricTextRole rhs) noexcept
    {
        return lhs | ToFlags(rhs);
    }

    [[nodiscard]] constexpr bool HasFlag(FLyricTextFlags flags, FLyricTextRole role) noexcept
    {
        return (flags & ToFlags(role)) != 0;
    }

    // =========================================================
    // 核心数据结构
    // =========================================================

    // 元数据 (如 Title, Artist, Album)
    struct DREAMLYRICPARSER_API FMetadata
    {
        std::map<std::string, std::string> items;

        // DLL 安全性：显式定义 Big 5 (析构/拷贝/移动)
        FMetadata();
        ~FMetadata();
        FMetadata(const FMetadata&);
        FMetadata& operator=(const FMetadata&);
        FMetadata(FMetadata&&) noexcept;
        FMetadata& operator=(FMetadata&&) noexcept;

        void Set(std::string key, std::string value);
        [[nodiscard]] std::optional<std::string> Get(const std::string& key) const;
    };

    // 单个词/字 (用于逐字歌词)
    struct DREAMLYRICPARSER_API FLyricWord
    {
        std::string text;
        FTimeSpan start_time;
        std::optional<FTimeSpan> end_time;
        FLyricTextRole role{FLyricTextRole::Lyric};

        // DLL 安全性
        FLyricWord();
        ~FLyricWord();
        FLyricWord(const FLyricWord&);
        FLyricWord& operator=(const FLyricWord&);
        FLyricWord(FLyricWord&&) noexcept;
        FLyricWord& operator=(FLyricWord&&) noexcept;
    };

    // 单行文本 (包含多个 Word)
    struct DREAMLYRICPARSER_API FLyricLine
    {
        FLyricTextRole role{FLyricTextRole::Lyric};
        std::string text;
        std::vector<FLyricWord> words;

        // DLL 安全性
        FLyricLine();
        ~FLyricLine();
        FLyricLine(const FLyricLine&);
        FLyricLine& operator=(const FLyricLine&);
        FLyricLine(FLyricLine&&) noexcept;
        FLyricLine& operator=(FLyricLine&&) noexcept;
    };

    // 歌词组 (同一时间点的所有行，如原文+翻译)
    struct DREAMLYRICPARSER_API FLyricGroup
    {
    public:
        FTimeSpan timestamp;
        FLyricTextFlags flags{ToFlags(FLyricTextRole::None)};

        FLyricGroup();
        ~FLyricGroup();
        FLyricGroup(const FLyricGroup&);
        FLyricGroup& operator=(const FLyricGroup&);
        FLyricGroup(FLyricGroup&&) noexcept;
        FLyricGroup& operator=(FLyricGroup&&) noexcept;

        void AddLine(FLyricLine line);
        [[nodiscard]] const std::vector<FLyricLine>& GetLines() const;

    private:
        std::vector<FLyricLine> lines;
    };

    // 最终解析结果
    struct DREAMLYRICPARSER_API FParsedLyric
    {
        FMetadata metadata;
        std::vector<FLyricGroup> groups;

        // DLL 安全性
        FParsedLyric();
        ~FParsedLyric();
        FParsedLyric(const FParsedLyric&);
        FParsedLyric& operator=(const FParsedLyric&);
        FParsedLyric(FParsedLyric&&) noexcept;
        FParsedLyric& operator=(FParsedLyric&&) noexcept;
    };

    // =========================================================
    // 辅助配置结构
    // =========================================================

    // 分组规则 (用于决定多行合并时的 Role 分配)
    struct DREAMLYRICPARSER_API FGroupingRule
    {
        std::vector<FLyricTextRole> sequence;
        FLyricTextRole fallback{FLyricTextRole::Lyric};

        // DLL 安全性
        FGroupingRule();
        ~FGroupingRule();
        FGroupingRule(const FGroupingRule&);
        FGroupingRule& operator=(const FGroupingRule&);
        FGroupingRule(FGroupingRule&&) noexcept;
        FGroupingRule& operator=(FGroupingRule&&) noexcept;

        // 工厂方法
        [[nodiscard]] static FGroupingRule Default();
    };

    struct DREAMLYRICPARSER_API FParserOptions
    {
        FGroupingRule grouping;

        // DLL 安全性
        FParserOptions();
        ~FParserOptions();
        FParserOptions(const FParserOptions&);
        FParserOptions& operator=(const FParserOptions&);
        FParserOptions(FParserOptions&&) noexcept;
        FParserOptions& operator=(FParserOptions&&) noexcept;
    };
} // namespace dream_lyric_parser

#ifdef _MSC_VER
#pragma warning(pop)
#endif
