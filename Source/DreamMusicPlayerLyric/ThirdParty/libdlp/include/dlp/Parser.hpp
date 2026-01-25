// Dream Lyric Parser Library CXX20
// Universal parser interface and base options
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <string_view>
#include <type_traits> // 补充必要的头文件

#include "Export.hpp"
#include "File.hpp"

namespace dlp::File
{
    struct FLyricWord;
}

namespace dlp::Parser
{
    /** * @brief Base class for parser configuration.
     */
    class DLP_API FParserOptions
    {
    public:
        FParserOptions() noexcept; // 构造函数通常不抛异常
        virtual ~FParserOptions();

        bool is_implemented = false;

        [[nodiscard]] static FParserOptions NoImplemented() noexcept;

        /**
         * @brief Factory method with support for constructor arguments.
         * @tparam T The specialized option type.
         * @tparam Args Argument types for T's constructor.
         */
        template <typename T, typename... Args>
        [[nodiscard]] static T* NewOption(Args&&... args)
        {
            static_assert(std::is_base_of_v<FParserOptions, T>, "T must be derived from FParserOptions");
            return new T(std::forward<Args>(args)...);
        }
    };

    /**
     * @brief Abstract base class (Interface) for all lyric parsers.
     */
    struct IParser
    {
    public:
        IParser() = default;
        explicit IParser(FParserOptions* in_parser_option) noexcept;
        virtual ~IParser();

    public:
        /**
         * @brief Primary entry point for parsing a full lyric document.
         */
        virtual void Parse(
            std::string_view in_lyric_string,
            File::FLyricGroupRoleOption in_role_option,
            std::vector<File::FLyricGroup>& out_groups,
            File::FLyricMetadata& out_metadata
        ) = 0;

        /**
         * @brief Parses a single raw string into a lyric line structure.
         */
        [[nodiscard]] virtual File::FLyricLine ParserLine(std::string_view in_string) = 0;

    protected:
        /**
         * @brief Extracts the starting timestamp from a line of text.
         */
        [[nodiscard]] virtual timestamp ParserLineStartTime(std::string_view in_string) const = 0;

        /**
         * @brief Decomposes a metadata line (ID-tag) into a Key-Value pair.
         */
        virtual void ParserMetadata(std::string_view in_string, std::string& out_key, std::string& out_content) = 0;

        /**
         * @brief Heuristic check to see if a line contains metadata tags or lyric text.
         */
        [[nodiscard]] virtual bool IsMetadataLine(std::string_view in_string) const noexcept = 0;

        FParserOptions* options = nullptr;
    };
}
