// Dream Lyric Parser Lib
// ESLyric (Enhanced LRC) Format Parser Implementation
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once

#include "dlp/Parser.hpp"

namespace dlp::Parser
{
    /**
     * @brief Parser for ESLyric and extended LRC formats.
     */
    struct DLP_API FParser_ESLyric final : public IParser
    {
    public:
        ~FParser_ESLyric() override;

        /**
         * @brief Parses the ESLyric/LRC content into structured groups.
         */
        void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) override;

        /**
         * @brief Parses a single line of LRC text.
         */
        [[nodiscard]] File::FLyricLine ParserLine(std::string_view in_string) override;

    protected:
        /** @brief Extracts the starting timestamp from the beginning of an LRC line. */
        [[nodiscard]] timestamp ParserLineStartTime(std::string_view in_string) const override;

        /** @brief Parses ID tags/metadata (e.g., [ti:Song Title], [ar:Artist]). */
        void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) override;

        /** @brief Checks if a line is a metadata tag rather than a lyric line. */
        [[nodiscard]] bool IsMetadataLine(std::string_view in_string) const noexcept override;
    };
}
