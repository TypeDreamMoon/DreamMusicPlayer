// Dream Lyric Parser Library CXX20
// Core type definitions and enumeration constants
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>

#include "Export.hpp"

/**
 * @namespace dlp
 * @brief Root namespace for the Dream Lyric Parser library.
 */
namespace dlp
{
    /** * @brief High-precision timestamp representation.
     * Uses std::chrono::duration with double-precision seconds to handle
     * sub-millisecond accuracy required by advanced formats.
     */
    typedef std::chrono::duration<double> timestamp;

    /** @brief Supported lyric and subtitle file formats. */
    enum class EFileFormat : int
    {
        /** Standard synchronized lyrics (.lrc) */
        LRC = 0,
        /** SubRip subtitle format (.srt) */
        SRT = 1,
        /** Enhanced word-by-word synchronized format */
        ESLyric = 2,
        /** Advanced Substation Alpha format (.ass/.ssa) */
        ASS = 3,
    };

    /** * @brief Categorization of lyric line content.
     * Designed as a bitmask to allow a single line to fulfill multiple roles if necessary.
     */
    enum class ELyricContentRole : std::uint8_t
    {
        /** Unclassified or default content */
        None = 0,
        /** Primary lyric text (Original language) */
        Lyric = 1 << 0,
        /** Phonetic transcription (e.g., Romaji, Pinyin) */
        Romanization = 1 << 1,
        /** Translated text */
        Translation = 1 << 2
    };

    /** @brief Type alias for combined ELyricContentRole bitwise flags. */
    using FLyricContentFlags = std::uint8_t;

    /**
     * @brief Converts a role enum value to its raw bitmask flag.
     * @param Role The specific role to convert.
     * @return The underlying bitmask value.
     */
    [[nodiscard]] constexpr FLyricContentFlags ToFlags(ELyricContentRole Role) noexcept
    {
        return static_cast<FLyricContentFlags>(Role);
    }

    /**
     * @brief Overloaded OR operator to combine roles into a flag set.
     */
    [[nodiscard]] constexpr FLyricContentFlags operator|(ELyricContentRole lhs, ELyricContentRole rhs) noexcept
    {
        return static_cast<FLyricContentFlags>(static_cast<FLyricContentFlags>(lhs) | static_cast<FLyricContentFlags>(rhs));
    }

    /**
     * @brief Overloaded OR operator to append a role to an existing flag set.
     */
    [[nodiscard]] constexpr FLyricContentFlags operator|(FLyricContentFlags lhs, ELyricContentRole rhs) noexcept
    {
        return lhs | ToFlags(rhs);
    }

    /**
     * @brief Utility to check if a flag set contains a specific content role.
     * @param flags The combined flag set.
     * @param role The role to search for.
     * @return True if the role bit is set.
     */
    [[nodiscard]] constexpr bool HasFlag(FLyricContentFlags flags, ELyricContentRole role) noexcept
    {
        return (flags & ToFlags(role)) != 0;
    }
}
