// Dream Lyric Parser Library CXX20
// Basic input string analysis
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <string_view>

#include "File.hpp"

namespace dlp
{
    /**
     * @brief Main processor class for handling lyric string analysis and conversion.
     */
    struct FProcess
    {
    public:
        FProcess() = default;
        virtual ~FProcess();

        /**
         * @brief Processes a raw lyric string into a structured lyric file object.
         *
         * @param in_file_format The format of the input lyric string.
         * @param in_lyric_string The string view containing the lyric data.
         * @param in_role_option Options for how roles/groups should be handled during processing.
         * @return File::FLyricFile The resulting structured lyric data.
         */
        virtual File::FLyricFile Process(EFileFormat in_file_format, std::string_view in_lyric_string, File::FLyricGroupRoleOption in_role_option);

    private:
        /**
         * @brief Retrieves the appropriate parser instance for the specified file format.
         *
         * @param in_file_format The format to get a parser for.
         * @return Parser::IParser* A pointer to the parser implementation.
         */
        static Parser::IParser* GetParser(EFileFormat in_file_format);
    };
}
