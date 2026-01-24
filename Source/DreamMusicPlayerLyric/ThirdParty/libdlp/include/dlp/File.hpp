// Dream Lyric Parser Library CXX20
// Unified lyrics file format
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <map>
#include <vector>

#include "Export.hpp"
#include "Types.hpp"

namespace dlp::Parser
{
    struct IParser;
}

namespace dlp::File
{
    /**
     * @brief Lyrics metadata structure for storing metadata information of lyric files
     * Stores key-value pairs of metadata such as song title, artist, etc.
     */
    struct DLP_API FLyricMetadata
    {
    public:
        /**
         * @brief Constructor, initializes the lyric metadata object
         */
        FLyricMetadata();

        /**
         * @brief Destructor, cleans up resources of the lyric metadata object
         */
        ~FLyricMetadata();

        /**
         * @brief Adds a metadata item
         * @param in_key Key name of the metadata
         * @param in_content Content of the metadata
         */
        void AddMetadata(const std::string& in_key, const std::string& in_content);

        /**
         * @brief Map storing lyric metadata, key is string type, value is also string type
         */
        std::map<std::string, std::string> metadata;
    };

    /**
     * @brief Lyric word structure, representing a single word in lyrics and its timestamp information
     */
    struct DLP_API FLyricWord
    {
    public:
        /**
         * @brief Constructor, initializes the lyric word object
         */
        FLyricWord();

        /**
         * @brief Destructor, cleans up resources of the lyric word object
         */
        ~FLyricWord();

        /**
         * @brief Stores the word text content
         */
        std::string word;

        /**
         * @brief Word start timestamp, default is zero
         */
        timestamp time_start = timestamp::zero();

        /**
         * @brief Word end timestamp, default is zero
         */
        timestamp time_end = timestamp::zero();
    };

    /**
     * @brief Lyric line structure, representing a line of lyrics and its parsed content
     */
    struct DLP_API FLyricLine
    {
    public:
        /**
         * @brief Constructor, initializes the lyric line object
         */
        FLyricLine();

        FLyricLine(std::string in_lyric, timestamp in_time_start, timestamp in_time_end);

        /**
         * @brief Virtual destructor, ensures proper destruction of derived classes
         */
        virtual ~FLyricLine();

        /**
         * @brief Parses the lyric line content, converting raw lyrics to parsed word list
         */
        void Parser();

        /**
         * @brief Checks if it is word-by-word lyric mode
         * @return Returns true if it is word-by-word lyric mode, otherwise returns false
         */
        [[nodiscard]] bool IsWordByWord() const;

        /**
         * @brief Lyric content role identifier, indicating the role type of this line of lyrics
         */
        ELyricContentRole role = ELyricContentRole::None;

        /**
         * @brief Parsed word list, containing timestamp information for each word
         */
        std::vector<FLyricWord> parsed_words;

        /**
         * @brief Original lyric text content
         */
        std::string lyric;

        /**
         * @brief Lyric line start timestamp, default is zero
         */
        timestamp time_start = timestamp::zero();

        /**
         * @brief Lyric line end timestamp, default is zero
         */
        timestamp time_end = timestamp::zero();

        /**
         * @brief Flag indicating whether it is word-by-word lyric mode
         */
        bool word_by_word = false;
    };

    /**
     * @brief Lyric group role option structure, defining role configuration when processing lyric groups
     */
    struct DLP_API FLyricGroupRoleOption
    {
        /**
         * @brief Constructor, initializes options with specified role list and fallback role
         * @param in_roles Role list
         * @param in_fallback Fallback role used when role list is empty
         */
        FLyricGroupRoleOption(std::vector<ELyricContentRole> in_roles, ELyricContentRole in_fallback);

        /**
         * @brief Stores the list of available roles
         */
        std::vector<ELyricContentRole> roles;

        /**
         * @brief Fallback role used when no suitable role is available
         */
        ELyricContentRole fallback;

        /**
         * @brief Applies role options to the specified vector of lyric lines
         * @param in_lines Vector of lyric lines to apply roles to
         */
        void ApplyRole(std::vector<FLyricLine>& in_lines) const;
    };

    /**
     * @brief Lyric group structure, managing a group of related lyric lines and providing parsing functionality
     */
    struct DLP_API FLyricGroup
    {
    public:
        /**
         * @brief Constructor, initializes the lyric group with specified parser, start time, and role options
         * @param in_start_time Start time of the lyric group
         * @param in_role_option Role options for the lyric group
         */
        FLyricGroup(timestamp in_start_time, FLyricGroupRoleOption in_role_option);

        /**
         * @brief Virtual destructor, ensures proper destruction of derived classes
         */
        virtual ~FLyricGroup();

        /**
         * @brief Processes the entire lyric group, executing parsing operations
         */
        void ProcessGroup();

        /**
         * @brief Checks if the current lyric group is valid
         * @return Returns true if the lyric group is valid, otherwise returns false
         */
        [[nodiscard]] bool IsValidGroup() const;

        /**
         * @brief Start timestamp of the lyric group
         */
        timestamp group_time_start = timestamp::zero();

        /**
         * @brief End timestamp of the lyric group
         */
        timestamp group_time_end = timestamp::zero();

        /**
         * @brief Role options for the lyric group
         */
        FLyricGroupRoleOption role_option;

        /**
         * @brief Stores the parsed lyric line list
         */
        std::vector<FLyricLine> parsed_lines;
    };

    /**
     * @brief Lyric file structure, representing a complete lyric file containing metadata and lyric groups
     */
    struct DLP_API FLyricFile
    {
    public:
        /**
         * @brief Default constructor, creates an empty lyric file object
         */
        FLyricFile();

        /**
         * @brief Constructor, initializes the lyric file with specified metadata and lyric group list
         * @param in_metadata Metadata of the lyric file
         * @param in_groups List of lyric groups in the lyric file
         */
        FLyricFile(const FLyricMetadata& in_metadata, const std::vector<FLyricGroup>& in_groups);

        /**
         * @brief Destructor, cleans up resources of the lyric file object
         */
        virtual ~FLyricFile();

        /**
         * @brief Metadata of the lyric file
         */
        FLyricMetadata metadata;

        /**
         * @brief List of lyric groups in the lyric file
         */
        std::vector<FLyricGroup> groups;
    };
}
