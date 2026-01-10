// DreamLyricParser - ASS parser
// SPDX-License-Identifier: MIT

#pragma once

#include "DreamLyricParser/Parser_Interface.hpp"

namespace dream_lyric_parser {

class DREAMLYRICPARSER_API FParserAss : public FParserLyric {
public:
    [[nodiscard]] bool CanParse(const std::string& content) const override;
    [[nodiscard]] FParsedLyric Parse(const std::string& content, const FParserOptions& options) const override;

private:
    static FParsedLyric ParseInternal(const std::string& content, const FParserOptions& options);
};

}  // namespace dream_lyric_parser

