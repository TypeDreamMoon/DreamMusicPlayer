// Dream Lyric Parser Lib
// LRC Format Parser
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"

namespace dlp::Parser
{
    struct DLP_API FParser_ESLyric : public IParser
    {
    public:
        ~FParser_ESLyric() override;

        void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) override;

        File::FLyricLine ParserLine(std::string_view in_string) override;

    protected:
        timestamp ParserLineStartTime(std::string_view in_string) override;
        void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) override;
        bool IsMetadataLine(std::string_view in_string) override;
    };
}