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
    virtual ~FParserLyric() = default;

    [[nodiscard]] virtual bool CanParse(const std::string& content) const = 0;
    [[nodiscard]] virtual FParsedLyric Parse(const std::string& content, const FParserOptions& options = FParserOptions{}) const = 0;
};

DREAMLYRICPARSER_API std::unique_ptr<FParserLyric> CreateParser(FParserFormat format);

}  // namespace dream_lyric_parser


