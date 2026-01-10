// DreamLyricParser - Constants
// SPDX-License-Identifier: MIT

#pragma once

namespace dream_lyric_parser
{
    namespace constants
    {
        namespace version
        {
            inline constexpr const char* VERSION = "0.2.0";
            inline const int MAJOR = 0;
            inline const int MINOR = 2;
            inline const int PATCH = 0;
            inline const bool IS_BETA = false;
            inline const bool IS_DEV = true;
            inline const bool IS_DEBUG = true;
            inline const bool IS_RELEASE = !IS_BETA && !IS_DEV && !IS_DEBUG;
            inline const char* BUILD_TIME = __TIME__;
            inline const char* BUILD_DATE = __DATE__;

        }// namespace version
    } // namespace constants
} // namespace dream_lyric_parser
