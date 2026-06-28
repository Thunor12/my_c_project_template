#ifndef NOB_BUILD_COMMON_H
#define NOB_BUILD_COMMON_H

#include <stddef.h>

/* Shared between nob_helpers.h, nob.c, and config/*.build.h feature modules. */

typedef enum
{
    COMP_RES__FAILED = 0,
    COMP_RES__NO_ACTION = 1,
    COMP_RES__OK = 2,
} CompResult;

typedef struct
{
    Nob_String_Builder *items;
    size_t count;
    size_t capacity;
} FileList;

struct NobBuildCtx;
typedef struct NobBuildCtx NobBuildCtx;

struct NobBuildCtx
{
    Nob_Procs *procs;
    const char *cc;
    const char *opt_level;
};

#endif
