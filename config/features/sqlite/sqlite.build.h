#ifndef NOB_CONFIG_SQLITE_BUILD_H
#define NOB_CONFIG_SQLITE_BUILD_H

/* Build logic for SQLite — include from nob.c after nob_helpers.h.
 * Requires config/enabled/sqlite.h (from config/features/sqlite/ via enable or copier). */

#ifndef NOB_CONFIG_HAS_SQLITE
#error "sqlite.build.h requires NOB_CONFIG_HAS_SQLITE from sqlite.h"
#endif

#include <ctype.h>
#include <string.h>

static bool sqlite_hex_eq(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
    }
    return *a == *b;
}

static bool sqlite_verify_sha256(const char *path, const char *expected_hex)
{
    const char *hash_path = BUILD_DIR ".verify.sha256";
    Nob_Cmd cmd = {0};

#ifdef _WIN32
    nob_cmd_append(&cmd, "powershell", "-NoProfile", "-Command",
                    "(Get-FileHash -LiteralPath", path, "-Algorithm SHA256).Hash.ToLower() | "
                    "Set-Content -NoNewline -Path", hash_path);
#else
    Nob_String_Builder sh = {0};
    nob_sb_appendf(&sh, "sha256sum '%s' | awk '{print $1}' > '%s'", path, hash_path);
    nob_sb_append_null(&sh);
    nob_cmd_append(&cmd, "sh", "-c", sh.items);
    nob_sb_free(sh);
#endif

    if (!nob_cmd_run(&cmd))
    {
        nob_log(NOB_ERROR, "Failed to compute SHA256 for %s", path);
        return false;
    }

    Nob_String_Builder hash_sb = {0};
    if (!nob_read_entire_file(hash_path, &hash_sb))
    {
        nob_delete_file(hash_path);
        nob_log(NOB_ERROR, "Failed to read SHA256 result for %s", path);
        return false;
    }
    nob_delete_file(hash_path);

    bool ok = sqlite_hex_eq(hash_sb.items, expected_hex);
    if (!ok)
        nob_log(NOB_ERROR, "SHA256 mismatch for %s (expected %s)", path, expected_hex);
    nob_sb_free(hash_sb);
    return ok;
}

static CompResult sqlite_fetch_amalgamation(void)
{
    if (nob_file_exists(SQLITE_SRC) && nob_file_exists(SQLITE_HDR))
    {
        if (!sqlite_verify_sha256(SQLITE_SRC, SQLITE_C_SHA256))
            return COMP_RES__FAILED;
        nob_log(NOB_INFO, "SQLite amalgamation already present");
        return COMP_RES__NO_ACTION;
    }

    nob_log(NOB_INFO, "Downloading SQLite amalgamation %s", SQLITE_VERSION);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
        return COMP_RES__FAILED;
    if (!nob_mkdir_if_not_exists(THIRD_PARTY_DIR))
        return COMP_RES__FAILED;
    if (!nob_mkdir_if_not_exists(SQLITE_DIR))
        return COMP_RES__FAILED;

    {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "curl", "-L", "-f", "-o", SQLITE_ZIP, SQLITE_URL);
        if (!nob_cmd_run(&cmd))
        {
            nob_log(NOB_ERROR, "Failed to download SQLite from %s", SQLITE_URL);
            nob_log(NOB_ERROR, "Install curl or place %s and %s manually", SQLITE_SRC, SQLITE_HDR);
            return COMP_RES__FAILED;
        }
    }

    if (!nob_extract_zip(SQLITE_ZIP, BUILD_DIR))
    {
        nob_log(NOB_ERROR, "Failed to extract %s (need unzip, python, or PowerShell)", SQLITE_ZIP);
        return COMP_RES__FAILED;
    }

    if (!nob_copy_file_in_dir(SQLITE_EXTRACT_DIR, SQLITE_DIR, "sqlite3.c") ||
        !nob_copy_file_in_dir(SQLITE_EXTRACT_DIR, SQLITE_DIR, "sqlite3.h"))
    {
        nob_log(NOB_ERROR, "Failed to copy SQLite sources into %s", SQLITE_DIR);
        return COMP_RES__FAILED;
    }

    nob_delete_file(SQLITE_ZIP);

    if (!nob_file_exists(SQLITE_SRC) || !nob_file_exists(SQLITE_HDR))
    {
        nob_log(NOB_ERROR, "SQLite sources missing after extraction");
        return COMP_RES__FAILED;
    }

    if (!sqlite_verify_sha256(SQLITE_SRC, SQLITE_C_SHA256))
        return COMP_RES__FAILED;

    nob_log(NOB_INFO, "SQLite amalgamation ready");
    return COMP_RES__OK;
}

static CompResult sqlite_compile_obj(const NobBuildCtx *ctx)
{
    CompResult dl = sqlite_fetch_amalgamation();
    if (dl == COMP_RES__FAILED)
        return COMP_RES__FAILED;

    const char *deps[] = {SQLITE_SRC, SQLITE_HDR};
    if (!nob_needs_rebuild(SQLITE_OBJ, deps, 2))
    {
        nob_log(NOB_INFO, "%s up to date", SQLITE_OBJ);
        return COMP_RES__NO_ACTION;
    }

    nob_log(NOB_INFO, "Compiling SQLite");
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, ctx->cc);
    nob_cmd_append(&cmd, ctx->opt_level, "-g", "-std=c17");
    nob_cmd_append(&cmd, "-Wno-unused-parameter");
    nob_cmd_append(&cmd, "-DSQLITE_THREADSAFE=0", "-DSQLITE_OMIT_LOAD_EXTENSION");
    nob_cmd_append(&cmd, "-c", SQLITE_SRC, "-o", SQLITE_OBJ);
    if (!nob_cmd_run(&cmd))
    {
        nob_log(NOB_ERROR, "Failed to compile SQLite");
        return COMP_RES__FAILED;
    }

    return COMP_RES__OK;
}

/* Download and verify amalgamation only (no compile). */
static CompResult sqlite_setup_deps(void)
{
    return sqlite_fetch_amalgamation();
}

/* Download (if needed) and compile sqlite3.o — library only, no examples. */
static CompResult sqlite_build_lib(const NobBuildCtx *ctx)
{
    return sqlite_compile_obj(ctx);
}

#endif
