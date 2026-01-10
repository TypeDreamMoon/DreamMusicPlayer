#pragma once

#include "DreamLyricParser/Parser.hpp"

namespace dream_lyric_parser::parser
{
    class DREAMLYRICPARSER_API Parser_ESLyric : public IParserLyric
    {
    public:
        Parser_ESLyric();
        virtual ~Parser_ESLyric();

        [[nodiscard]] bool CanParse(const FLyricString& content) const override;
        [[nodiscard]] FParsedLyric Parse(const FLyricString& content, const FParserOptions& options) const override;
    };
}