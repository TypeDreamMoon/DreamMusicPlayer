// Dream Lyric Parser Library CXX20
// ASS Parser Implementation (Fixed for Romaji grouping)
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"
#include <vector>
#include <string_view>

namespace dlp::Parser
{
    class FParser_ASS final : public IParser
    {
    public:
        FParser_ASS() = default;
        ~FParser_ASS() override = default;

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

    private:
        // --- Internal Helpers ---

        static timestamp ParseTimestamp(std::string_view time_str);
        static void ParseKaraoke(std::string_view text, std::vector<File::FLyricWord>& out_words, timestamp line_start);
        static std::string StripTags(std::string_view text);
        static size_t FindNthComma(std::string_view str, int n);

        static ELyricContentRole GuessRoleFromStyle(std::string_view style_name);

        static void SortLinesByRole(std::vector<File::FLyricLine>& lines, const std::vector<ELyricContentRole>& target_roles);
    };
}
