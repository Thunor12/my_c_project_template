#ifndef NOB_CONFIG_TEAPOT_BUILD_H
#define NOB_CONFIG_TEAPOT_BUILD_H

/* Build logic for teapot — include from nob.c after nob_helpers.h. */

#ifndef NOB_CONFIG_HAS_TEAPOT
#error "teapot.build.h requires NOB_CONFIG_HAS_TEAPOT from teapot.h"
#endif

static CompResult teapot_setup_deps(void)
{
    if (nob_file_exists(TEAPOT_HDR))
    {
        nob_log(NOB_INFO, "teapot already present at %s", TEAPOT_HDR);
        return COMP_RES__NO_ACTION;
    }

    nob_log(NOB_INFO, "Fetching teapot submodule (%s)", TEAPOT_SUBMODULE);

    {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "git", "submodule", "update", "--init", "--depth", "1", TEAPOT_SUBMODULE);
        if (nob_cmd_run(&cmd) && nob_file_exists(TEAPOT_HDR))
            return COMP_RES__OK;
    }

    nob_log(NOB_ERROR, "teapot sources missing at %s", TEAPOT_HDR);
    nob_log(NOB_ERROR, "Ensure .gitmodules lists %s, then: git submodule update --init", TEAPOT_SUBMODULE);
    nob_log(NOB_ERROR, "Or run: scripts/enable-teapot.sh");
    return COMP_RES__FAILED;
}

static CompResult teapot_build_lib(FileList *out_obj)
{
    CompResult setup = teapot_setup_deps();
    if (setup == COMP_RES__FAILED)
        return COMP_RES__FAILED;

    if (!nob_file_exists(TEAPOT_HDR))
    {
        nob_log(NOB_INFO, "teapot header missing — run: ./nob setup teapot");
        return COMP_RES__NO_ACTION;
    }

    return compile_stblib_to_obj(TEAPOT_HDR, TEAPOT_STBLIB_IMPL, out_obj);
}

#endif
