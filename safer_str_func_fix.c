//
// Created by Jackie Yang on 2/4/21.
//
#include "safer_str_func_fix.h"

#ifndef __STDC_LIB_EXT1__
errno_t strcat_s(char *restrict dest, rsize_t destsz, const char *restrict src) {
    strcat(dest, src);
    return 0;
}
#endif