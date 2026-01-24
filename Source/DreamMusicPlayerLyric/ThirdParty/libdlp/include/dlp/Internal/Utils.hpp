// Dream Lyric Parser Lib
// Internal Utils
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once
#include <string_view>
#include <vector>

namespace dlp::Internal::Utils
{
    std::vector<std::string_view> SplitLines(std::string_view in_lyric_string);
    std::string_view NormalizeLine(std::string_view s);
}
