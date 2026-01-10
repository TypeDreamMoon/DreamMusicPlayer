#pragma once

#include "DreamLyricParser/Parser.hpp"

namespace dream_lyric_parser::parser
{
    class DREAMLYRICPARSER_API Parser_YRC : public IParserLyric
    {
    public:
        Parser_YRC();
        virtual ~Parser_YRC();

        [[nodiscard]] bool CanParse(const FLyricString& content) const override;
        [[nodiscard]] FParsedLyric Parse(const FLyricString& content, const FParserOptions& options) const override;
    };
}