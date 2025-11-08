// DreamLyricParser - Parser interface
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include "DreamLyricParser/Export.hpp"
#include "DreamLyricParser/Types.hpp"

namespace dream_lyric_parser {

enum class FParserFormat {
    LrcLineByLine,
    LrcWordByWord,
    LrcEsLyric,
    Srt,
    Ass
};

class DREAMLYRICPARSER_API FParserLyric {
public:
    // 显式虚析构函数，确保在 DLL 中正确释放
    // 这对于跨 DLL 边界使用非常重要
    virtual ~FParserLyric();

    [[nodiscard]] virtual bool CanParse(const std::string& content) const = 0;
    
    // 移除默认参数，改为重载函数以避免跨 DLL 边界问题
    [[nodiscard]] virtual FParsedLyric Parse(const std::string& content, const FParserOptions& options) const = 0;
    
    // 重载函数，提供默认选项
    [[nodiscard]] FParsedLyric Parse(const std::string& content) const {
        return Parse(content, FParserOptions{});
    }
};

DREAMLYRICPARSER_API std::unique_ptr<FParserLyric> CreateParser(FParserFormat format);

}  // namespace dream_lyric_parser


