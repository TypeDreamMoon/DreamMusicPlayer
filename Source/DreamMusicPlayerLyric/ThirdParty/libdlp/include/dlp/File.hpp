// Dream Lyric Parser Library CXX20
// Unified lyrics file format definitions
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <map>
#include <vector>
#include <string>

#include "Export.hpp"
#include "Types.hpp"

namespace dlp::Parser
{
    struct IParser;
}

namespace dlp::File
{
    /**
     * @brief Container for lyric file metadata.
     * Stores key-value pairs such as title (ti), artist (ar), album (al), and time offset.
     */
    struct DLP_API FLyricMetadata
    {
    public:
        FLyricMetadata();
        ~FLyricMetadata();

        /**
         * @brief Inserts or updates a metadata entry.
         * @param in_key Attribute name (e.g., "artist").
         * @param in_content Attribute value content.
         */
        void AddMetadata(const std::string& in_key, const std::string& in_content);

        /** @brief Internal storage map for metadata tags. */
        std::map<std::string, std::string> metadata;
    };

    /**
     * @brief Represents a single word or syllable within a lyric line.
     * Used for "karaoke-style" or word-by-word synchronized lyrics.
     */
    struct DLP_API FLyricWord
    {
    public:
        FLyricWord() noexcept;
        ~FLyricWord();

        /** @brief Start timestamp relative to the beginning of the song. */
        timestamp time_start = timestamp::zero();
        /** @brief End timestamp relative to the beginning of the song. */
        timestamp time_end = timestamp::zero();
        /** @brief Text content of the word (e.g., a single character or word). */
        std::string word;
    };

    /**
     * @brief Represents a single line of lyrics and its synchronization data.
     */
    struct DLP_API FLyricLine
    {
    public:
        FLyricLine();
        FLyricLine(std::string in_lyric, timestamp in_time_start, timestamp in_time_end);
        virtual ~FLyricLine();

        /**
         * @brief Parses raw lyric text into a list of words.
         * Fills parsed_words if the format supports word-level timestamps.
         */
        void Parser();

        /** @brief Checks if the line contains word-level synchronization. */
        [[nodiscard]] bool IsWordByWord() const noexcept;

        /** @brief Start timestamp for the line to appear. */
        timestamp time_start = timestamp::zero();
        /** @brief End timestamp for the line to disappear. */
        timestamp time_end = timestamp::zero();
        /** @brief Identifies the role of the line (e.g., Main, Romaji, Translation). */
        ELyricContentRole role = ELyricContentRole::None;
        /** @brief Flag indicating if word-by-word mode is active. */
        bool word_by_word = false;

        /** @brief Raw text content of the lyric line. */
        std::string lyric;
        /** @brief List of parsed words with individual timestamps if word_by_word is true. */
        std::vector<FLyricWord> parsed_words;
    };

    /**
     * @brief Configuration for assigning roles to lines within a group.
     * Defines how the parser distinguishes between primary lyrics and translations.
     */
    struct DLP_API FLyricGroupRoleOption
    {
        FLyricGroupRoleOption();
        FLyricGroupRoleOption(std::vector<std::vector<ELyricContentRole>> in_roles, ELyricContentRole in_fallback);

        /** @brief Applies role assignment logic to a vector of lyric lines. */
        void ApplyRole(std::vector<FLyricLine>& in_lines) const;

        /** @brief Clears existing role matching rules. */
        FLyricGroupRoleOption* ClearRoles() noexcept;
        /** @brief Adds a priority-ordered role matching group. */
        FLyricGroupRoleOption* AddRoleGroup(const std::vector<ELyricContentRole>& in_role_group);
        /** @brief Sets the fallback role when no rules match. */
        FLyricGroupRoleOption* SetFallbackRole(ELyricContentRole in_fallback_role) noexcept;

        /** @brief Priority-ordered list of available roles. */
        std::vector<std::vector<ELyricContentRole>> roles;
        /** @brief Default role used as a fallback. */
        ELyricContentRole fallback = ELyricContentRole::None;

        static FLyricGroupRoleOption Default();
    };

    /**
     * @brief Manages a collection of related lyric lines.
     * A "group" usually represents different versions of a line at the same time (e.g., Original + Romaji + Translation).
     */
    struct DLP_API FLyricGroup
    {
    public:
        FLyricGroup(timestamp in_start_time, FLyricGroupRoleOption in_role_option);
        virtual ~FLyricGroup();

        /** @brief Executes role assignment and timestamp validation within the group. */
        void ProcessGroup();

        /** @brief Validates if the group contains usable lyric content. */
        [[nodiscard]] bool IsValidGroup() const noexcept;

        /** @brief Overall start time for the lyric group. */
        timestamp group_time_start = timestamp::zero();
        /** @brief Overall end time (determined by the longest line in the group). */
        timestamp group_time_end = timestamp::zero();

        /** @brief Role identification configuration used by this group. */
        FLyricGroupRoleOption role_option;
        /** @brief List of lyric lines within this group (e.g., line 1 is Main, line 2 is Trans). */
        std::vector<FLyricLine> parsed_lines;
    };

    /**
     * @brief Top-level representation of a parsed lyric file.
     * Encapsulates all metadata and synchronized lyric groups.
     */
    struct DLP_API FLyricFile
    {
    public:
        FLyricFile();
        FLyricFile(const FLyricMetadata& in_metadata, const std::vector<FLyricGroup>& in_groups);
        virtual ~FLyricFile();

        /** @brief Metadata for the song (Artist, Title, etc.). */
        FLyricMetadata metadata;
        /** @brief List of lyric groups sorted by time. */
        std::vector<FLyricGroup> groups;
    };
}
