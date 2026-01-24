// Dream Lyric Parser Library CXX20
// Universal parser type
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <string_view>

#include "Export.hpp"
#include "File.hpp"

namespace dlp::File
{
    struct FLyricWord;
}

namespace dlp::Parser
{
    /** @brief Structure containing parsed lyric data */
    struct DLP_API FParsedData
    {
        ~FParsedData();
    };

    /** @brief Defines the grouping role for the parser */
    struct DLP_API FParserGroupingRole
    {
        ~FParserGroupingRole();
    };

    /** @brief Options for configuring the parser behavior */
    struct DLP_API FParserOptions
    {
        ~FParserOptions();
    };

    /**
     * @brief Interface for lyric parsers
     */
    struct IParser
    {
    public:
        IParser() = default;
        virtual ~IParser() = default;

    public:
        /**
         * @brief Parses a lyric string into groups and metadata
         * @param in_lyric_string The raw lyric string to parse
         * @param in_role_option Options for grouping roles
         * @param out_groups Output vector for parsed lyric groups
         * @param out_metadata Output structure for parsed metadata
         */
        virtual void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) = 0;

        /**
         * @brief Parses a single line of lyrics
         * @param in_string The line string to parse
         * @return The parsed lyric line structure
         */
        virtual File::FLyricLine ParserLine(std::string_view in_string) = 0;

    protected:
        /**
         * @brief Extracts the start time from a lyric line string
         * @param in_string The line string
         * @return The start timestamp
         */
        virtual timestamp ParserLineStartTime(std::string_view in_string) = 0;

        /**
         * @brief Parses a metadata line into key and content
         * @param in_string The metadata line string
         * @param out_key Output string for the metadata key
         * @param out_content Output string for the metadata content
         */
        virtual void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) = 0;

        /**
         * @brief Checks if a line is a metadata line
         * @param in_string The line string to check
         * @return True if it's a metadata line, false otherwise
         */
        virtual bool IsMetadataLine(std::string_view in_string) = 0;
    };
}
