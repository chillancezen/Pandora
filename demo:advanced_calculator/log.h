#ifndef _LOG_H
#define _LOG_H
#include <stdio.h>
#include <stdlib.h>
//#define _SILENT

#ifndef _SILENT
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) 
#endif


#define ASSERT(cond, ...) { \
    if (!(cond)) { \
        printf("\x1b[31m"); \
        printf(__VA_ARGS__); \
        printf("\x1b[0m"); \
        exit(1); \
    } \
}


#endif
