// DreamLyricParser - Common lyric data structures
// SPDX-License-Identifier: MIT

#pragma once

// Suppress C4251 warning: STL containers in exported classes are expected
// This is safe because we ensure proper memory management with explicit destructors
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

namespace dream_lyric_parser {

struct DREAMLYRICPARSER_API FTimeSpan {
    int hours{0};
    int minutes{0};
    int seconds{0};
    int milliseconds{0};

    constexpr FTimeSpan() = default;
    FTimeSpan(int h, int m, int s, int ms) noexcept;

    [[nodiscard]] int64_t ToTotalMilliseconds() const noexcept;
    [[nodiscard]] static FTimeSpan FromTotalMilliseconds(int64_t total_ms) noexcept;
    [[nodiscard]] static FTimeSpan Parse(const std::string& text);
    [[nodiscard]] std::string ToString(bool include_hours = false, int fractional_digits = 3) const;

    FTimeSpan& Normalize() noexcept;
    [[nodiscard]] FTimeSpan Normalized() const noexcept;

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

enum class FLyricTextRole : std::uint8_t {
    None = 0,
    Lyric = 1 << 0,
    Romanization = 1 << 1,
    Translation = 1 << 2
};

using FLyricTextFlags = std::uint8_t;

[[nodiscard]] constexpr FLyricTextFlags ToFlags(FLyricTextRole role) noexcept {
    return static_cast<FLyricTextFlags>(role);
}

[[nodiscard]] constexpr FLyricTextFlags operator|(FLyricTextRole lhs, FLyricTextRole rhs) noexcept {
    return static_cast<FLyricTextFlags>(static_cast<FLyricTextFlags>(lhs) | static_cast<FLyricTextFlags>(rhs));
}

[[nodiscard]] constexpr FLyricTextFlags operator|(FLyricTextFlags lhs, FLyricTextRole rhs) noexcept {
    return lhs | ToFlags(rhs);
}

[[nodiscard]] constexpr bool HasFlag(FLyricTextFlags flags, FLyricTextRole role) noexcept {
    return (flags & ToFlags(role)) != 0;
}

struct DREAMLYRICPARSER_API FMetadata {
    std::map<std::string, std::string> items;

    // 显式析构函数，确保在 DLL 中正确释放 STL 容器
    ~FMetadata();

    void Set(std::string key, std::string value);
    [[nodiscard]] std::optional<std::string> Get(const std::string& key) const;
};

struct DREAMLYRICPARSER_API FLyricWord {
    std::string text;
    FTimeSpan start_time;
    std::optional<FTimeSpan> end_time;
    FLyricTextRole role{FLyricTextRole::Lyric};

    // 显式析构函数，确保在 DLL 中正确释放 std::string
    ~FLyricWord();
};

struct DREAMLYRICPARSER_API FLyricLine {
    FLyricTextRole role{FLyricTextRole::Lyric};
    std::string text;
    std::vector<FLyricWord> words;

    // 显式析构函数，确保在 DLL 中正确释放 STL 容器
    ~FLyricLine();
};

struct DREAMLYRICPARSER_API FLyricGroup {
    FTimeSpan timestamp;
    std::vector<FLyricLine> lines;
    FLyricTextFlags flags{ToFlags(FLyricTextRole::None)};

    // 显式析构函数，确保在 DLL 中正确释放 STL 容器
    ~FLyricGroup();

    void AddLine(FLyricLine line);
};

struct DREAMLYRICPARSER_API FParsedLyric {
    FMetadata metadata;
    std::vector<FLyricGroup> groups;

    // 默认构造函数
    FParsedLyric();
    
    // 复制构造函数和赋值运算符，确保正确的内存管理
    FParsedLyric(const FParsedLyric&);
    FParsedLyric& operator=(const FParsedLyric&);
    
    // 移动构造函数和赋值运算符
    FParsedLyric(FParsedLyric&&) noexcept;
    FParsedLyric& operator=(FParsedLyric&&) noexcept;

    // 显式析构函数，确保在 DLL 中正确释放 STL 容器
    // 这对于跨 DLL 边界使用非常重要
    ~FParsedLyric();
};

struct DREAMLYRICPARSER_API FGroupingRule {
    std::vector<FLyricTextRole> sequence;
    FLyricTextRole fallback{FLyricTextRole::Lyric};

    // 默认构造函数
    FGroupingRule();
    
    // 复制构造函数和赋值运算符
    FGroupingRule(const FGroupingRule&);
    FGroupingRule& operator=(const FGroupingRule&);
    
    // 移动构造函数和赋值运算符
    FGroupingRule(FGroupingRule&&) noexcept;
    FGroupingRule& operator=(FGroupingRule&&) noexcept;

    // 显式析构函数，确保在 DLL 中正确释放 STL 容器
    ~FGroupingRule();

    // 返回默认分组规则，确保在 DLL 中正确构造
    [[nodiscard]] static FGroupingRule Default();
};

struct DREAMLYRICPARSER_API FParserOptions {
    FGroupingRule grouping;
    
    // 默认构造函数，在 DLL 中正确初始化
    FParserOptions();
    
    // 复制构造函数和赋值运算符
    FParserOptions(const FParserOptions&);
    FParserOptions& operator=(const FParserOptions&);
    
    // 移动构造函数和赋值运算符
    FParserOptions(FParserOptions&&) noexcept;
    FParserOptions& operator=(FParserOptions&&) noexcept;
};

}  // namespace dream_lyric_parser

#ifdef _MSC_VER
#pragma warning(pop)
#endif

