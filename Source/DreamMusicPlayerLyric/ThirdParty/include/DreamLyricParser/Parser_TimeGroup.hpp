// DreamLyricParser - Time grouping utilities
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "DreamLyricParser/Export.hpp"
#include "DreamLyricParser/Types.hpp"

namespace dream_lyric_parser {

class DREAMLYRICPARSER_API FParserTimeGroup {
public:
    explicit FParserTimeGroup(FGroupingRule rule);

    void AddLine(const FTimeSpan& timestamp, FLyricLine line);
    [[nodiscard]] std::vector<FLyricGroup> Consume();

private:
    FGroupingRule rule_;
    std::vector<FLyricGroup> groups_;
    std::map<int64_t, std::size_t> index_lookup_;

    FLyricTextRole ResolveRole(const FLyricGroup& group, const FLyricLine& line) const;
};

}  // namespace dream_lyric_parser


