#ifndef NOB_CONFIG_H
#define NOB_CONFIG_H

/* Core paths — always present in every generated project. */
#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"
#define TEST_DIR "./tests/"
#define EXAMPLE_DIR "./examples/"
#define THIRD_PARTY_DIR "./third_party/"

#define NOB_SUBMODULE_DIR THIRD_PARTY_DIR "nob.h/"
#define NOB_HDR NOB_SUBMODULE_DIR "nob.h"

/* Toolchain — override in nob.c before including nob_helpers.h if needed. */
#ifdef _WIN32
#define CC "gcc.exe"
#define EXE_SUFFIX ".exe"
#else
#define CC "gcc"
#define EXE_SUFFIX ""
#endif

#define OPT_LEVEL "-O2"
#define THIRD_PARTY_COMPILE_FLAGS OPT_LEVEL

#ifndef COMPILE_FLAGS_BASE
#define COMPILE_FLAGS_BASE OPT_LEVEL, "-g",                                                         \
                           "-I" SRC_DIR,                                                          \
                           "-I" THIRD_PARTY_DIR,                                                  \
                           "-Wall", "-Wextra", "-Wpedantic", "-Wswitch-enum", "-Wconversion", "-Wimplicit-fallthrough", \
                           "-Wshadow", "-Wpointer-arith", "-Wcast-qual", "-Wstrict-prototypes",   \
                           "-D_FORTIFY_SOURCE=2",                                                 \
                           "-fstack-clash-protection",                                            \
                           "-fstack-protector-strong",                                            \
                           "-std=c17"
#endif

/* Optional feature configs:
 one header per feature in config/enabled/.
 * Added at template render time (copier) or via scripts/enable-*.sh */
#if __has_include("config/enabled/sqlite.h")
#include "config/enabled/sqlite.h"
#endif

#if __has_include("config/enabled/teapot.h")
#include "config/enabled/teapot.h"
#endif

#endif
