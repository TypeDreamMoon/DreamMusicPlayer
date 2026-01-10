// DreamLyricParser - Export macros
// SPDX-License-Identifier: MIT

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef DREAMLYRICPARSER_BUILD_SHARED
    #ifdef DREAMLYRICPARSER_EXPORTS
      #define DREAMLYRICPARSER_API __declspec(dllexport)
    #else
      #define DREAMLYRICPARSER_API __declspec(dllimport)
    #endif
  #else
    #define DREAMLYRICPARSER_API
  #endif
#else
  #if __GNUC__ >= 4
    #define DREAMLYRICPARSER_API __attribute__((visibility("default")))
  #else
    #define DREAMLYRICPARSER_API
  #endif
#endif


