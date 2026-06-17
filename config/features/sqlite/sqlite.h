#ifndef NOB_CONFIG_SQLITE_H
#define NOB_CONFIG_SQLITE_H

#define NOB_CONFIG_HAS_SQLITE 1

#define SQLITE_VERSION "3450100"
#define SQLITE_URL "https://www.sqlite.org/2024/sqlite-amalgamation-" SQLITE_VERSION ".zip"
#define SQLITE_DIR THIRD_PARTY_DIR "sqlite/"
#define SQLITE_ZIP BUILD_DIR "sqlite-amalgamation-" SQLITE_VERSION ".zip"
#define SQLITE_EXTRACT_DIR BUILD_DIR "sqlite-amalgamation-" SQLITE_VERSION
#define SQLITE_SRC SQLITE_DIR "sqlite3.c"
#define SQLITE_HDR SQLITE_DIR "sqlite3.h"
#define SQLITE_OBJ BUILD_DIR "sqlite3.o"
#define SQLITE_EXAMPLE EXAMPLE_DIR "sqlite_demo.c"
/* SHA-256 of sqlite3.c (verify after download) — see dep.toml */
#define SQLITE_C_SHA256 "1a206854aa9fe0ccc1b609f5cfce67eb52ac0a8f5aa2f5e853f7c3ed84c710ab"

#endif
