// Dream Lyric Parser Library CXX20
// Common lyric data structures
// Copyright (C) 2026 Type Dream Moon. All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <chrono>

#include "Export.hpp"

/**
 * @namespace dlp
 * @brief Main namespace for the Dream Lyric Parser library.
 */
namespace dlp
{
    /** @brief Represents a timestamp in seconds. */
    typedef std::chrono::duration<double> timestamp;

    /** @brief Supported lyric file formats. */
    enum class EFileFormat : int
    {
        /** Standard LRC Format */
        LRC = 0,
        /** SubRip Text */
        SRT = 1,
        /** ESLyric Word-by-word Format */
        ESLyric = 2,
        /** ASS Subtitle Format */
        ASS = 3,
    };

    /** @brief Roles that a lyric content line can represent. */
    enum class ELyricContentRole : std::uint8_t
    {
        /** No role */
        None = 0,
        /** Lyric role*/
        Lyric = 1 << 0,
        /** Romanization of the lyrics*/
        Romanization = 1 << 1,
        /** Translation of the lyrics*/
        Translation = 1 << 2
    };

    /** @brief Bitwise flags for ELyricContentRole. */
    using FLyricContentFlags = std::uint8_t;

    /**
     * @brief Converts a single role to its bitwise flag representation.
     * @param Role The role to convert.
     * @return The flag representation.
     */
    [[nodiscard]] constexpr FLyricContentFlags ToFlags(ELyricContentRole Role) noexcept
    {
        return static_cast<FLyricContentFlags>(Role);
    }

    /**
     * @brief Combines two roles into a flag set.
     * @param lhs Left-hand side role.
     * @param rhs Right-hand side role.
     * @return Combined flags.
     */
    [[nodiscard]] constexpr FLyricContentFlags operator|(ELyricContentRole lhs, ELyricContentRole rhs) noexcept
    {
        return static_cast<FLyricContentFlags>(static_cast<FLyricContentFlags>(lhs) | static_cast<FLyricContentFlags>(rhs));
    }

    /**
     * @brief Adds a role to an existing flag set.
     * @param lhs Existing flags.
     * @param rhs Role to add.
     * @return Combined flags.
     */
    [[nodiscard]] constexpr FLyricContentFlags operator|(FLyricContentFlags lhs, ELyricContentRole rhs) noexcept
    {
        return lhs | ToFlags(rhs);
    }

    /**
     * @brief Checks if a specific role is present in the flags.
     * @param flags The flags to check.
     * @param role The role to look for.
     * @return True if the role is present.
     */
    [[nodiscard]] constexpr bool HasFlag(FLyricContentFlags flags, ELyricContentRole role) noexcept
    {
        return (flags & ToFlags(role)) != 0;
    }
}
