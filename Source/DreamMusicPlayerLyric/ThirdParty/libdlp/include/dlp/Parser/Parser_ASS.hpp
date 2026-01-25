// Dream Lyric Parser Library CXX20
// ASS (Advanced Substation Alpha) Parser Implementation
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"
#include <vector>
#include <string_view>

namespace dlp::Parser
{
    /**
     * @brief Configuration options for the ASS parser.
     */
    class FParserOptions_ASS : public FParserOptions
    {
    public:
        FParserOptions_ASS(const std::string& in_key_lyric, const std::string& in_key_romanization, const std::string& in_key_translation) noexcept
            : key_lyric(in_key_lyric), key_romanization(in_key_romanization), key_translation(in_key_translation)
        {
            is_implemented = true;
        }

        std::string key_lyric = "lrc";
        std::string key_romanization = "roma";
        std::string key_translation = "trans";
    };

    /**
     * @brief Final implementation of the ASS (Advanced Substation Alpha) lyric parser.
     */
    class FParser_ASS final : public IParser
    {
    public:
        FParser_ASS() = default;
        FParser_ASS(FParserOptions* in_parser_option) noexcept;
        ~FParser_ASS() override = default;

        /**
         * @brief Parses the entire ASS script content into structured lyric groups.
         */
        void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) override;

        /**
         * @brief Parses a single "Dialogue" line into a FLyricLine object.
         */
        [[nodiscard]] File::FLyricLine ParserLine(std::string_view in_string) override;

    protected:
        // 解析类辅助接口应标记为 const，保证解析过程中不意外修改 Parser 状态
        [[nodiscard]] timestamp ParserLineStartTime(std::string_view in_string) const override;

        void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) override;

        [[nodiscard]] bool IsMetadataLine(std::string_view in_string) const noexcept override;

    private:
        // --- Internal Helpers ---

        /** @brief Converts ASS timestamp format (H:MM:SS.cc) to internal timestamp type. */
        [[nodiscard]] static timestamp ParseTimestamp(std::string_view time_str) noexcept;

        /** @brief Extracts karaoke tags (e.g., {\k50}) and words from the text string. */
        static void ParseKaraoke(std::string_view text, std::vector<File::FLyricWord>& out_words, timestamp line_start);

        /** @brief Removes override tags (everything inside {}) from the string. */
        [[nodiscard]] static std::string StripTags(std::string_view text);

        /** @brief Locates the position of the N-th comma. */
        [[nodiscard]] static size_t FindNthComma(std::string_view str, int n) noexcept;

        /** @brief Determines the lyric role based on the style name defined in options. */
        [[nodiscard]] ELyricContentRole GuessRoleFromStyle(std::string_view style_name) const noexcept;
    };
}
