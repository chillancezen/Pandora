#ifndef _LOG_H
#define _LOG_H
#include <stdio.h>

//#define _SILENT

#ifndef _SILENT
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) 
#endif

#endif
