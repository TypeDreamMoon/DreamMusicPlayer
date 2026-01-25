// Dream Lyric Parser Lib
// SRT (SubRip) Format Parser
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"

namespace dlp::Parser
{
    /**
     * @brief Parser for SRT (SubRip) subtitle files.
     * Handles sequential blocks containing an index, a time range (00:00:00,000),
     * and one or more lines of text.
     */
    struct FParser_SRT final : public IParser
    {
    public:
        ~FParser_SRT() override;

        /**
         * @brief Parses the raw SRT content.
         * Iterates through subtitle blocks and converts them into lyric groups.
         */
        void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) override;

        /**
         * @brief Parses a single line within the SRT structure.
         * @note For SRT, this is typically called internally by Parse().
         */
        [[nodiscard]] File::FLyricLine ParserLine(std::string_view in_string) override;

    protected:
        /** * @brief Extracts the start time from an SRT timestamp line.
         * Converts "HH:MM:SS,mmm" format into internal timestamp.
         */
        [[nodiscard]] timestamp ParserLineStartTime(std::string_view in_string) const override;

        /** * @brief Parses metadata if present.
         */
        void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) override;

        /** * @brief Identifies if a line is structural (like the numeric index or blank line).
         */
        [[nodiscard]] bool IsMetadataLine(std::string_view in_string) const noexcept override;
    };
}
