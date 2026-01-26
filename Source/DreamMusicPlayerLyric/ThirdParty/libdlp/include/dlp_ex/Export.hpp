#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef LIBDLP_EX_BUILD_SHARED
    #ifdef DLP_EX_EXPORT
      #define DLP_EX_API __declspec(dllexport)
    #else
      #define DLP_EX_API __declspec(dllimport)
    #endif
  #else
    #define DLP_EX_API
  #endif
#else
  #if __GNUC__ >= 4
    #define DLP_EX_API __attribute__((visibility("default")))
  #else
    #define DLP_EX_API
  #endif
#endif