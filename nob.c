// Skeleton build script — what you ship in generated projects.
// Bootstrap: gcc -o nob nob.c  (or ./nob.sh / nob.cmd)
//
// Template repo maintainers dogfood the full catalog with nob.template.c instead.
#define NOB_IMPLEMENTATION
#include "third_party/nob.h/nob.h"

#define NOB_HELPERS_IMPLEMENTATION
#include "nob_helpers.h"

#ifdef NOB_CONFIG_HAS_SQLITE
#include "config/enabled/sqlite.build.h"
#endif

#ifdef NOB_CONFIG_HAS_TEAPOT
#include "config/enabled/teapot.build.h"
#endif

#include "config/nob_macros.h"
#include "config/nob_dep_setup.h"
#include "config/nob_dep_examples.h"

int main(int argc, char **argv)
{
    int ret = 0;
    CompResult comp_res = COMP_RES__NO_ACTION;
    int need_sync = 0;
    int do_tests = 0;
    int do_examples = 0;
    int do_setup_all = 0;

    Nob_File_Paths sources = {0};

    NOB_GO_REBUILD_URSELF_PLUS(argc, argv, NOB_HDR, "nob_config.h", "config/build_common.h",
                               "nob_helpers.h", "config/nob_macros.h", "config/nob_dep_setup.h",
                               "config/nob_dep_examples.h");

    if (!nob_file_exists(NOB_HDR))
    {
        nob_log(NOB_ERROR, "nob.h submodule missing.");
        nob_log(NOB_ERROR, "Run: git submodule update --init third_party/nob.h");
        return 1;
    }

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return 1;

    nob_shift_args(&argc, &argv);

    while (argc > 0)
    {
        char *arg = nob_shift_args(&argc, &argv);
        if (0 == strcmp(arg, "clean"))
        {
            nob_log(NOB_INFO, "Cleaning...");
            if (!nob_clean_build_dir(BUILD_DIR))
                return 1;
            return 0;
        }
        if (0 == strcmp(arg, "test") || 0 == strcmp(arg, "tests"))
            do_tests = 1;
        if (0 == strcmp(arg, "example") || 0 == strcmp(arg, "examples"))
            do_examples = 1;
        if (0 == strcmp(arg, "setup"))
        {
            if (argc > 0)
            {
                char *what = nob_shift_args(&argc, &argv);
                nob_log(NOB_ERROR, "Unknown setup target: %s — use: ./nob setup", what);
                return 1;
            }
            do_setup_all = 1;
        }
    }

    if (do_setup_all)
    {
        comp_res = nob_run_setup_all();
        HANDLE_COM_RES(comp_res, need_sync);
        goto defer;
    }

    if (do_tests)
    {
        nob_da_append(&sources, TEST_DIR "test_smoke.c");
        comp_res = build_program("test_smoke", &sources, &(FileList){0}, NULL);
        HANDLE_COM_RES(comp_res, need_sync);

        if (run_built_exe("test_smoke") != 0)
            ret = 1;
        goto defer;
    }

    if (do_examples)
    {
        int built_any = 0;

        nob_da_append(&sources, EXAMPLE_DIR "hello.c");
        comp_res = build_program("hello", &sources, &(FileList){0}, NULL);
        MARK_BUILT(comp_res, built_any);
        HANDLE_COM_RES(comp_res, need_sync);

        comp_res = nob_build_dep_examples(&sources, &built_any);
        HANDLE_COM_RES(comp_res, need_sync);

        if (!built_any)
        {
            nob_log(NOB_WARNING, "No examples were built.");
        }

        goto defer;
    }

    nob_da_append(&sources, SRC_DIR "main.c");
    comp_res = build_program("main", &sources, &(FileList){0}, NULL);
    HANDLE_COM_RES(comp_res, need_sync);

defer:
    if (comp_res == COMP_RES__FAILED)
        ret = 1;

    nob_procs_flush(&procs);
    nob_da_free(sources);
    return ret;
}
