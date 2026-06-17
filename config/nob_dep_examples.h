#ifndef NOB_DEP_EXAMPLES_H
#define NOB_DEP_EXAMPLES_H

/* Build optional dependency demo binaries — include inside the examples target. */

static CompResult nob_build_dep_examples(Nob_File_Paths *sources, int *built_any)
{
    CompResult comp_res = COMP_RES__NO_ACTION;

#ifdef NOB_CONFIG_HAS_SQLITE
    if (nob_file_exists(SQLITE_EXAMPLE))
    {
        comp_res = sqlite_build_lib(&g_build);
        if (comp_res == COMP_RES__FAILED)
            return COMP_RES__FAILED;

        sources->count = 0;
        nob_da_append(sources, SQLITE_EXAMPLE);
        comp_res = build_program("sqlite_demo", sources, &(FileList){0}, SQLITE_OBJ);
        MARK_BUILT(comp_res, *built_any);
        if (comp_res == COMP_RES__FAILED)
            return COMP_RES__FAILED;
    }
#endif

#ifdef NOB_CONFIG_HAS_TEAPOT
    if (nob_file_exists(TEAPOT_EXAMPLE))
    {
        FileList teapot_objs = {0};
        comp_res = teapot_build_lib(&teapot_objs);
        if (comp_res == COMP_RES__FAILED)
            return COMP_RES__FAILED;

        sources->count = 0;
        nob_da_append(sources, TEAPOT_EXAMPLE);
        comp_res = build_program("teapot_hello", sources, &teapot_objs, NULL);
        MARK_BUILT(comp_res, *built_any);
        free_file_list(&teapot_objs);
        if (comp_res == COMP_RES__FAILED)
            return COMP_RES__FAILED;
    }
#endif

    return comp_res;
}

#endif
