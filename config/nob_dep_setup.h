#ifndef NOB_DEP_SETUP_H
#define NOB_DEP_SETUP_H

/* Optional dependency setup — include from nob.c after feature *.build.h headers. */

static CompResult nob_run_setup_all(void)
{
    CompResult result = COMP_RES__NO_ACTION;
    CompResult step = COMP_RES__NO_ACTION;

#ifdef NOB_CONFIG_HAS_SQLITE
    step = sqlite_setup_deps();
    if (step == COMP_RES__FAILED)
        return COMP_RES__FAILED;
    if (step == COMP_RES__OK)
        result = COMP_RES__OK;
#endif

#ifdef NOB_CONFIG_HAS_TEAPOT
    step = teapot_setup_deps();
    if (step == COMP_RES__FAILED)
        return COMP_RES__FAILED;
    if (step == COMP_RES__OK)
        result = COMP_RES__OK;
#endif

#ifndef NOB_CONFIG_HAS_SQLITE
#ifndef NOB_CONFIG_HAS_TEAPOT
    nob_log(NOB_WARNING, "No optional dependencies enabled in this project.");
    nob_log(NOB_WARNING, "Enable features: scripts/enable-*.sh — or: copier copy with include_* flags");
#endif
#endif

    return result;
}

#endif
