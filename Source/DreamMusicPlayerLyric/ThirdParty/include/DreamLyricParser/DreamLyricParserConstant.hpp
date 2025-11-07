// DreamLyricParser - Constants and regex patterns
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace dream_lyric_parser {
namespace constants {

// LRC format patterns
inline constexpr const char* LRC_TIME_PATTERN = R"(\[(\d{1,2}):(\d{2})[.:](\d{2,3})\])";
inline constexpr const char* LRC_TIME_PATTERN_STRICT = R"(\[(\d{2}):(\d{2})[.:](\d{2,3})\])";
inline constexpr const char* LRC_INLINE_TIMESTAMP_PATTERN = R"(\[(\d{1,2}):(\d{2})[.:](\d{2,3})\]([^\[]+))";
inline constexpr const char* LRC_WORD_PATTERN = R"(<([^>]+)>\s*([^<]+))";
inline constexpr const char* LRC_RANGE_PATTERN = R"(<([^>]+)>\s*<([^>]+)>\s*([^<]+))";
inline constexpr const char* LRC_TAG_PATTERN = R"(\[[^\]]*\])";

// SRT format patterns
inline constexpr const char* SRT_TIMESTAMP_LINE_PATTERN = R"((\d{2}:\d{2}:\d{2}[,.]\d{3})\s*-->\s*(\d{2}:\d{2}:\d{2}[,.]\d{3}))";
inline constexpr const char* SRT_TIMESTAMP_PATTERN = R"((\d{1,2}):(\d{2}):(\d{2})[,.](\d{3}))";
inline constexpr const char* SRT_SEQUENCE_PATTERN = R"(\d+)";

// ASS format patterns
inline constexpr const char* ASS_DIALOGUE_PATTERN = R"(Dialogue:\s*[^,]*,\s*([^,]+),\s*([^,]+),\s*[^,]*,\s*[^,]*,\s*[^,]*,\s*[^,]*,\s*[^,]*,\s*[^,]*,\s*(.+))";
inline constexpr const char* ASS_TIMESTAMP_PATTERN = R"((\d{1,2}):(\d{1,2}):(\d{1,2})[.:](\d{1,2}))";
inline constexpr const char* ASS_KARAOKE_PATTERN = R"(\{\\kf(\d+)\}([^\}]*))";
inline constexpr const char* ASS_EVENTS_SECTION = R"(\[Events\])";

// Metadata tags
inline const std::vector<std::string> LRC_METADATA_TAGS = {
    "ar", "ti", "al", "by", "offset", "tool", "re", "ve"
};

// ASS style types
inline constexpr const char* ASS_STYLE_ORIG = "orig";
inline constexpr const char* ASS_STYLE_TS = "ts";
inline constexpr const char* ASS_STYLE_ROMA = "roma";

// Default durations (milliseconds)
inline constexpr int64_t DEFAULT_WORD_DURATION_MS = 500;
inline constexpr int64_t DEFAULT_LINE_DURATION_MS = 3000;

// Separators
inline constexpr char METADATA_SEPARATOR = ':';
inline constexpr char SRT_TIMESTAMP_SEPARATOR[] = " --> ";

}  // namespace constants
}  // namespace dream_lyric_parser

