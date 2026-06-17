#ifndef NOB_HELPERS_H
#define NOB_HELPERS_H

/* Reusable build helpers — include after nob.h.
 * In exactly one translation unit: #define NOB_HELPERS_IMPLEMENTATION before this include. */

#include "nob_config.h"
#include "config/build_common.h"
#include <string.h>

extern Nob_Procs procs;
extern NobBuildCtx g_build;

void free_file_list(FileList *fl);
CompResult compile_stblib_to_obj(const char *header_path, const char *impl_flag, FileList *out_objs);
CompResult build_program(const char *exe_name, Nob_File_Paths *sources, FileList *stb_objs, const char *extra_obj);
int run_built_exe(const char *exe_name);

/* Cross-platform filesystem helpers (nob.h under the hood). */
bool nob_extract_zip(const char *zip_path, const char *dest_dir);
bool nob_copy_file_in_dir(const char *src_dir, const char *dst_dir, const char *filename);

#ifdef NOB_HELPERS_IMPLEMENTATION

Nob_Procs procs = {0};
NobBuildCtx g_build = {
    .procs = &procs,
    .cc = CC,
    .opt_level = OPT_LEVEL,
};

static bool nob_path_has_sep(const char *dir)
{
    size_t n = strlen(dir);
    return n > 0 && (dir[n - 1] == '/' || dir[n - 1] == '\\');
}

bool nob_copy_file_in_dir(const char *src_dir, const char *dst_dir, const char *filename)
{
    Nob_String_Builder src = {0};
    Nob_String_Builder dst = {0};
    if (nob_path_has_sep(src_dir))
        nob_sb_appendf(&src, "%s%s", src_dir, filename);
    else
        nob_sb_appendf(&src, "%s/%s", src_dir, filename);
    if (nob_path_has_sep(dst_dir))
        nob_sb_appendf(&dst, "%s%s", dst_dir, filename);
    else
        nob_sb_appendf(&dst, "%s/%s", dst_dir, filename);
    nob_sb_append_null(&src);
    nob_sb_append_null(&dst);

    bool ok = nob_copy_file(src.items, dst.items);
    nob_sb_free(src);
    nob_sb_free(dst);
    return ok;
}

bool nob_extract_zip(const char *zip_path, const char *dest_dir)
{
    Nob_Cmd cmd = {0};

#ifdef _WIN32
    nob_cmd_append(&cmd, "powershell", "-NoProfile", "-Command",
                    "Expand-Archive", "-LiteralPath", zip_path,
                    "-DestinationPath", dest_dir, "-Force");
    if (nob_cmd_run(&cmd))
        return true;
    nob_cmd_free(cmd);
    cmd = (Nob_Cmd){0};
#endif

    nob_cmd_append(&cmd, "unzip", "-o", zip_path, "-d", dest_dir);
    if (nob_cmd_run(&cmd))
        return true;
    nob_cmd_free(cmd);
    cmd = (Nob_Cmd){0};

    nob_cmd_append(&cmd, "python3", "-c",
                    "import zipfile,sys; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])",
                    zip_path, dest_dir);
    if (nob_cmd_run(&cmd))
        return true;
    nob_cmd_free(cmd);
    cmd = (Nob_Cmd){0};

    nob_cmd_append(&cmd, "python", "-c",
                    "import zipfile,sys; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])",
                    zip_path, dest_dir);
    return nob_cmd_run(&cmd);
}

static void append_compile_flags(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, COMPILE_FLAGS_BASE);
#ifdef NOB_CONFIG_HAS_SQLITE
    nob_cmd_append(cmd, "-I", SQLITE_DIR);
#endif
#ifdef NOB_CONFIG_HAS_TEAPOT
    nob_cmd_append(cmd, "-I", TEAPOT_DIR);
#endif
}

static void append_link_flags(Nob_Cmd *cmd)
{
    nob_cmd_append(cmd, "-O2", "-I", SRC_DIR, "-I", THIRD_PARTY_DIR);
#ifdef NOB_CONFIG_HAS_SQLITE
    nob_cmd_append(cmd, "-I", SQLITE_DIR);
#endif
#ifdef NOB_CONFIG_HAS_TEAPOT
    nob_cmd_append(cmd, "-I", TEAPOT_DIR);
#endif
#ifdef _WIN32
    nob_cmd_append(cmd, "-lws2_32");
#endif
}

void free_file_list(FileList *fl)
{
    if (!fl)
        return;
    for (size_t i = 0; i < fl->count; i++)
        nob_sb_free(fl->items[i]);
}

static void src_to_output_name(Nob_String_Builder *sb, const char *src_path, const char *ext)
{
    nob_sb_appendf(sb, BUILD_DIR "%s%s", nob_path_name(src_path), ext);
    nob_sb_append_null(sb);
}

static CompResult compile_objs(Nob_File_Paths *sources, FileList *out_objs)
{
    CompResult result = COMP_RES__NO_ACTION;
    Nob_Cmd cmd = {0};

    for (size_t i = 0; i < sources->count; i++)
    {
        Nob_String_Builder obj = {0};
        src_to_output_name(&obj, sources->items[i], ".o");

        if (out_objs)
            nob_da_append(out_objs, obj);

        if (!nob_needs_rebuild(obj.items, &sources->items[i], 1))
        {
            nob_log(NOB_INFO, "%s up to date", obj.items);
            continue;
        }

        nob_cmd_append(&cmd, CC);
        append_compile_flags(&cmd);
        nob_cc_output(&cmd, obj.items);
        nob_cmd_append(&cmd, "-c");
        nob_cc_inputs(&cmd, sources->items[i]);
        append_link_flags(&cmd);

        if (!nob_cmd_run(&cmd, .async = &procs))
        {
            result = COMP_RES__FAILED;
            goto defer;
        }
        result = COMP_RES__OK;
    }

defer:
    nob_cmd_free(cmd);
    return result;
}

CompResult compile_stblib_to_obj(const char *header_path, const char *impl_flag, FileList *out_objs)
{
    CompResult result = COMP_RES__OK;
    Nob_Cmd cmd = {0};

    if (nob_get_file_type(header_path) != NOB_FILE_REGULAR)
    {
        nob_log(NOB_WARNING, "stb header not found, skipping: %s", header_path);
        result = COMP_RES__NO_ACTION;
        goto defer;
    }

    Nob_String_Builder impl_flag_sb = {0};
    nob_sb_appendf(&impl_flag_sb, "-D%s", impl_flag);

    Nob_String_Builder obj = {0};
    src_to_output_name(&obj, header_path, ".o");

    if (out_objs)
        nob_da_append(out_objs, obj);

    if (!nob_needs_rebuild(obj.items, &header_path, 1))
    {
        nob_log(NOB_INFO, "%s up to date", obj.items);
        result = COMP_RES__NO_ACTION;
        goto defer;
    }

    nob_cmd_append(&cmd, CC);
    nob_cmd_append(&cmd, "-x", "c");
    nob_cmd_append(&cmd, THIRD_PARTY_COMPILE_FLAGS, impl_flag_sb.items);
    nob_cc_output(&cmd, obj.items);
    nob_cmd_append(&cmd, "-c");
    nob_cc_inputs(&cmd, header_path);
    append_link_flags(&cmd);

    if (!nob_cmd_run(&cmd, .async = &procs))
    {
        result = COMP_RES__FAILED;
        goto defer;
    }

defer:
    nob_cmd_free(cmd);
    nob_sb_free(impl_flag_sb);
    return result;
}

static CompResult compile_exe(const char *exe_name, Nob_File_Paths *deps)
{
    CompResult result = COMP_RES__OK;
    Nob_Cmd cmd = {0};
    Nob_String_Builder out = {0};
    nob_sb_appendf(&out, BUILD_DIR "%s%s", exe_name, EXE_SUFFIX);
    nob_sb_append_null(&out);

    nob_log(NOB_INFO, "Linking [%s]", out.items);

    if (!nob_needs_rebuild(out.items, deps->items, deps->count))
    {
        nob_log(NOB_INFO, "%s up to date", out.items);
        result = COMP_RES__NO_ACTION;
        goto defer;
    }

    nob_cmd_append(&cmd, CC);
    append_compile_flags(&cmd);
    nob_cmd_append(&cmd, "-o", out.items);
    for (size_t i = 0; i < deps->count; i++)
        nob_cmd_append(&cmd, deps->items[i]);
    append_link_flags(&cmd);

    if (!nob_cmd_run(&cmd, .async = &procs))
        result = COMP_RES__FAILED;

defer:
    nob_cmd_free(cmd);
    nob_sb_free(out);
    return result;
}

CompResult build_program(const char *exe_name, Nob_File_Paths *sources, FileList *stb_objs, const char *extra_obj)
{
    CompResult result = COMP_RES__OK;
    FileList objs = {0};
    Nob_File_Paths link_deps = {0};

    result = compile_objs(sources, &objs);
    if (result == COMP_RES__FAILED)
        goto defer;

    if (!nob_procs_flush(&procs))
    {
        result = COMP_RES__FAILED;
        goto defer;
    }

    for (size_t i = 0; i < stb_objs->count; i++)
        nob_da_append(&link_deps, stb_objs->items[i].items);
    for (size_t i = 0; i < objs.count; i++)
        nob_da_append(&link_deps, objs.items[i].items);
    if (extra_obj)
        nob_da_append(&link_deps, extra_obj);

    result = compile_exe(exe_name, &link_deps);
    if (result == COMP_RES__FAILED)
        goto defer;

    if (!nob_procs_flush(&procs))
        result = COMP_RES__FAILED;

defer:
    nob_da_free(link_deps);
    free_file_list(&objs);
    return result;
}

int run_built_exe(const char *exe_name)
{
    Nob_String_Builder out = {0};
    nob_sb_appendf(&out, BUILD_DIR "%s%s", exe_name, EXE_SUFFIX);
    nob_sb_append_null(&out);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, out.items);
    int ok = nob_cmd_run(&cmd);

    nob_cmd_free(cmd);
    nob_sb_free(out);
    return ok ? 0 : 1;
}

#endif /* NOB_HELPERS_IMPLEMENTATION */

#endif /* NOB_HELPERS_H */
