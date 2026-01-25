// Dream Lyric Parser Lib
// LRC Format Parser
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"

namespace dlp::Parser
{
    /**
     * @brief Standard LRC (LyRiC) format parser.
     * Handles basic synchronized lyrics with [mm:ss.xx] timestamps.
     */
    struct FParser_LRC final : public IParser
    {
    public:
        ~FParser_LRC() override;

        /**
         * @brief Parses the raw LRC input string.
         */
        void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) override;

    protected:
        /**
         * @brief Parses a single line of LRC text into a structured lyric line.
         */
        [[nodiscard]] File::FLyricLine ParserLine(std::string_view in_string) override;

        /**
         * @brief Extracts the start time from an LRC line (e.g., from "[01:23.45] text").
         */
        [[nodiscard]] timestamp ParserLineStartTime(std::string_view in_string) const override;

        /**
         * @brief Parses an LRC metadata tag (e.g., [ar:Artist Name]).
         */
        void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) override;

        /**
         * @brief Determines if a line is a metadata header (ID-tag) or a lyric line.
         */
        [[nodiscard]] bool IsMetadataLine(std::string_view in_string) const noexcept override;
    };
}
