// Dream Lyric Parser Lib
// Export Macros
// Copyright 2026 Dream Moon. All rights reserved.

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef LIBDLP_BUILD_SHARED
    #ifdef DLP_API
      #define DLP_API __declspec(dllexport)
    #else
      #define DLP_API __declspec(dllimport)
    #endif
  #else
    #define DLP_API
  #endif
#else
  #if __GNUC__ >= 4
    #define DLP_API __attribute__((visibility("default")))
  #else
    #define DLP_API
  #endif
#endif
