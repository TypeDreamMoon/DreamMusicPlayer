// Dream Lyric Parser Library CXX20
// Main entry point for lyric processing and orchestration
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <string_view>

#include "File.hpp"

namespace dlp
{
    namespace Parser
    {
        class FParserOptions;
        struct IParser;
    }

    /**
     * @brief Orchestrator class for analyzing and converting raw lyric strings.
     * Acts as the primary interface for the library, managing the lifecycle
     * of specific parser implementations.
     */
    struct FProcess
    {
    public:
        FProcess() = default;
        virtual ~FProcess();

        /**
         * @brief Converts a raw lyric string into a structured FLyricFile object.
         *
         * @param in_file_format The specific lyric format (LRC, SRT, ASS, etc.).
         * @param in_lyric_string The raw text content to be analyzed.
         * @param in_role_option Logic configuration for lyric roles (e.g., Main vs. Translation).
         * @param in_parser_option Optional format-specific settings (use nullptr for defaults).
         * @return File::FLyricFile A complete object containing metadata and synchronized groups.
         */
        static File::FLyricFile Process(
            EFileFormat in_file_format,
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            Parser::FParserOptions* in_parser_option = nullptr
        );

        /**
         * @brief Converts a raw lyric string into a structured FLyricFile object.
         *
         * @param in_file_format The specific lyric format (LRC, SRT, ASS, etc.).
         * @param in_lyric_string The raw text content to be analyzed.
         * @param in_parser_option Optional format-specific settings (use nullptr for defaults).
         * @return File::FLyricFile A complete object containing metadata and synchronized groups.
         */
        static File::FLyricFile Process(
            EFileFormat in_file_format,
            std::string_view in_lyric_string,
            Parser::FParserOptions* in_parser_option = nullptr
        );

    private:
        /**
         * @brief Internal Factory method to retrieve the correct parser implementation.
         *
         * @param in_file_format The format identifier.
         * @param in_parser_option Configuration for the requested parser.
         * @return Parser::IParser* A pointer to the concrete parser (e.g., FParser_LRC).
         */
        static Parser::IParser* GetParser(EFileFormat in_file_format, Parser::FParserOptions* in_parser_option = nullptr);
    };
}
