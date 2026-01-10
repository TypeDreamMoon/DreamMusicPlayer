#pragma once

#include "DreamLyricParser/Parser.hpp"

namespace dream_lyric_parser::parser
{
    class DREAMLYRICPARSER_API Parser_LRC : public IParserLyric
    {
    public:
        Parser_LRC();
        virtual ~Parser_LRC();

        // 简单判断是否包含 LRC 特征字符
        [[nodiscard]] bool CanParse(const FLyricString& content) const override;

        // 核心解析逻辑
        [[nodiscard]] FParsedLyric Parse(const FLyricString& content, const FParserOptions& options) const override;
    };
}