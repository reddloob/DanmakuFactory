//
// Created by Jackie Yang on 2/4/21.
//
#ifndef DANMAKUFACTORY_SAFER_STR_FUNC_FIX_H
#define DANMAKUFACTORY_SAFER_STR_FUNC_FIX_H

#include <string.h>

#ifdef __STDC_LIB_EXT1__
#define  __STDC_WANT_LIB_EXT1__ 1
#else
typedef int rsize_t;
typedef int errno_t;

errno_t strcat_s(char *restrict dest, rsize_t destsz, const char *restrict src);
#endif

#endif //DANMAKUFACTORY_SAFER_STR_FUNC_FIX_H
