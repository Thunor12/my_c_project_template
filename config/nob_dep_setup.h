#ifndef NOB_DEP_SETUP_H
#define NOB_DEP_SETUP_H

/* Optional dependency setup — generated from config/catalog.yaml + optional_deps. */

static CompResult nob_run_setup_all(void)
{
    CompResult result = COMP_RES__NO_ACTION;
    CompResult step = COMP_RES__NO_ACTION;


    step = sqlite_setup_deps();
    if (step == COMP_RES__FAILED)
        return COMP_RES__FAILED;
    if (step == COMP_RES__OK)
        result = COMP_RES__OK;

    step = teapot_setup_deps();
    if (step == COMP_RES__FAILED)
        return COMP_RES__FAILED;
    if (step == COMP_RES__OK)
        result = COMP_RES__OK;



    return result;
}

#endif
