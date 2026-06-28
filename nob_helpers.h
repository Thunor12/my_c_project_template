#ifndef NOB_HELPERS_H
#define NOB_HELPERS_H

/* Reusable build helpers — include after nob.h.
 * In exactly one translation unit: #define NOB_HELPERS_IMPLEMENTATION before this include. */

#include "nob_config.h"
#include "config/build_common.h"
#include <string.h>
#include <stdlib.h>

extern Nob_Procs procs;
extern NobBuildCtx g_build;

void free_file_list(FileList *fl);
CompResult compile_stblib_to_obj(const char *header_path, const char *impl_flag, FileList *out_objs);
CompResult build_program(const char *exe_name, Nob_File_Paths *sources, FileList *stb_objs, const char *extra_obj);
int run_built_exe(const char *exe_name);

/* Cross-platform filesystem helpers (nob.h under the hood). */
bool nob_extract_zip(const char *zip_path, const char *dest_dir);
bool nob_copy_file_in_dir(const char *src_dir, const char *dst_dir, const char *filename);
bool nob_clean_build_dir(const char *dir);
void nob_go_rebuild_urself_project(int argc, char **argv, const char *source_path);

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

bool nob_clean_build_dir(const char *dir)
{
    Nob_Cmd cmd = {0};

#ifdef _WIN32
    nob_cmd_append(&cmd, "powershell", "-NoProfile", "-Command",
                    "Remove-Item", "-LiteralPath", dir, "-Recurse", "-Force",
                    "-ErrorAction", "SilentlyContinue");
#else
    nob_cmd_append(&cmd, "rm", "-rf", dir);
#endif

    return nob_cmd_run(&cmd);
}

void nob_go_rebuild_urself_project(int argc, char **argv, const char *source_path)
{
    const char *fixed_deps[] = {
        NOB_HDR,
        "nob_config.h",
        "config/build_common.h",
        "nob_helpers.h",
        "config/nob_macros.h",
        "config/nob_dep_setup.h",
        "config/nob_dep_examples.h",
    };

    const char *binary_path = nob_shift(argv, argc);
#ifdef _WIN32
    if (!nob_sv_ends_with_cstr(nob_sv_from_cstr(binary_path), ".exe"))
        binary_path = nob_temp_sprintf("%s.exe", binary_path);
#endif

    Nob_File_Paths source_paths = {0};
    nob_da_append(&source_paths, source_path);
    for (size_t i = 0; i < NOB_ARRAY_LEN(fixed_deps); i++)
        nob_da_append(&source_paths, fixed_deps[i]);

    Nob_File_Paths enabled_entries = {0};
    if (nob_get_file_type("config/enabled") == NOB_FILE_DIRECTORY &&
        nob_read_entire_dir("config/enabled", &enabled_entries))
    {
        for (size_t i = 0; i < enabled_entries.count; i++)
        {
            const char *name = enabled_entries.items[i];
            size_t n = strlen(name);

            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
                continue;
            if (n < 3 || strcmp(name + n - 2, ".h") != 0)
                continue;

            nob_da_append(&source_paths, nob_temp_sprintf("config/enabled/%s", name));
        }
    }

    int rebuild_is_needed = nob_needs_rebuild(binary_path, source_paths.items, source_paths.count);
    nob_da_free(source_paths);
    if (rebuild_is_needed < 0)
        exit(1);
    if (!rebuild_is_needed)
        return;

    Nob_Cmd cmd = {0};
    const char *old_binary_path = nob_temp_sprintf("%s.old", binary_path);

    if (!nob_rename(binary_path, old_binary_path))
        exit(1);
    nob_cmd_append(&cmd, NOB_REBUILD_URSELF(binary_path, source_path));
    Nob_Cmd_Opt opt = {0};
    if (!nob_cmd_run_opt(&cmd, opt))
    {
        nob_rename(old_binary_path, binary_path);
        exit(1);
    }

    nob_cmd_append(&cmd, binary_path);
    nob_da_append_many(&cmd, argv, argc);
    if (!nob_cmd_run_opt(&cmd, opt))
        exit(1);
    exit(0);
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
    free(fl->items);
    fl->items = NULL;
    fl->count = 0;
    fl->capacity = 0;
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
