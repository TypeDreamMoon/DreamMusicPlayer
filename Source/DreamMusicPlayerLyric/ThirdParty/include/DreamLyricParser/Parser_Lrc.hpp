// DreamLyricParser - LRC parser variants
// SPDX-License-Identifier: MIT

#pragma once

#include "DreamLyricParser/Parser_Interface.hpp"
#include "DreamLyricParser/Parser_TimeGroup.hpp"

namespace dream_lyric_parser {

class DREAMLYRICPARSER_API FParserLrcLineByLine : public FParserLyric {
public:
    [[nodiscard]] bool CanParse(const std::string& content) const override;
    [[nodiscard]] FParsedLyric Parse(const std::string& content, const FParserOptions& options = FParserOptions{}) const override;

private:
    static FParsedLyric ParseInternal(const std::string& content, const FParserOptions& options);
};

class DREAMLYRICPARSER_API FParserLrcWordByWord : public FParserLyric {
public:
    [[nodiscard]] bool CanParse(const std::string& content) const override;
    [[nodiscard]] FParsedLyric Parse(const std::string& content, const FParserOptions& options = FParserOptions{}) const override;

private:
    static FParsedLyric ParseInternal(const std::string& content, const FParserOptions& options);
};

class DREAMLYRICPARSER_API FParserLrcEsLyric : public FParserLyric {
public:
    [[nodiscard]] bool CanParse(const std::string& content) const override;
    [[nodiscard]] FParsedLyric Parse(const std::string& content, const FParserOptions& options = FParserOptions{}) const override;

private:
    static FParsedLyric ParseInternal(const std::string& content, const FParserOptions& options);
};

}  // namespace dream_lyric_parser


