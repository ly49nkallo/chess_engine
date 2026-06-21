/*
 * bot_registry.c -- discover bot executables at runtime.
 *
 * Scans the "bots/" subdirectory that sits next to the running engine
 * binary.  Each file whose name ends in ".exe" (Windows) or which is
 * marked executable (POSIX) is registered as a bot.
 */

#include "bot_registry.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#  define BOT_REG_WINDOWS 1
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  define BOT_REG_POSIX 1
#  include <dirent.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

/* ------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------ */

/* Strip the .exe suffix (in-place, case-insensitive). */
static void strip_exe(char *name)
{
    int len = (int)strlen(name);
    if (len > 4
        && (name[len-4] == '.' )
        && ((name[len-3] == 'e') || (name[len-3] == 'E'))
        && ((name[len-2] == 'x') || (name[len-2] == 'X'))
        && ((name[len-1] == 'e') || (name[len-1] == 'E'))) {
        name[len-4] = '\0';
    }
}

/* get_exe_path -- full path of the running executable */
void get_exe_path(char *buf, int size)
{
#ifdef BOT_REG_WINDOWS
    if (GetModuleFileNameA(NULL, buf, (DWORD)size) == 0)
        strncpy(buf, ".", (size_t)size);
#else
    ssize_t len = readlink("/proc/self/exe", buf, (size_t)(size - 1));
    if (len > 0) buf[len] = '\0'; else strncpy(buf, ".", (size_t)size);
#endif
}

/* Find the directory containing the running executable. */
static void get_exe_dir(char *buf, int size)
{
    get_exe_path(buf, size);
    /* Strip the filename component */
    char *last = buf;
    for (char *p = buf; *p; p++) {
        if (*p == '\\' || *p == '/') last = p;
    }
    *last = '\0';
    if (buf[0] == '\0') strncpy(buf, ".", (size_t)size);
}

/* ------------------------------------------------------------------
 * bot_registry_create
 * ------------------------------------------------------------------ */
BotRegistry *bot_registry_create(void)
{
    BotRegistry *reg = (BotRegistry *)malloc(sizeof(BotRegistry));
    if (!reg) return NULL;
    reg->count = 0;

    /* Locate the bots/ directory next to this binary. */
    char exe_dir[BOT_PATH_MAX];
    get_exe_dir(exe_dir, (int)sizeof(exe_dir));

    char bots_dir[BOT_PATH_MAX];
    snprintf(bots_dir, sizeof(bots_dir), "%s%cbots", exe_dir,
#ifdef BOT_REG_WINDOWS
             '\\'
#else
             '/'
#endif
    );

#ifdef BOT_REG_WINDOWS
    /* Use FindFirstFile / FindNextFile to enumerate the bots/ directory. */
    char pattern[BOT_PATH_MAX];
    snprintf(pattern, sizeof(pattern), "%s\\*.exe", bots_dir);

    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        /* bots/ dir not found or empty -- not an error */
        return reg;
    }
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (reg->count >= BOT_REGISTRY_MAX) break;

        BotInfo *b = &reg->bots[reg->count];
        strncpy(b->name, ffd.cFileName, BOT_NAME_MAX - 1);
        b->name[BOT_NAME_MAX - 1] = '\0';
        strip_exe(b->name);

        snprintf(b->exe_path, BOT_PATH_MAX, "%s\\%s", bots_dir, ffd.cFileName);

        reg->count++;
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);

#else /* POSIX */
    DIR *d = opendir(bots_dir);
    if (!d) return reg; /* bots/ dir not present -- not an error */

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL && reg->count < BOT_REGISTRY_MAX) {
        if (entry->d_name[0] == '.') continue; /* skip . and .. */

        char full[BOT_PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", bots_dir, entry->d_name);

        /* Only register executable regular files. */
        struct stat st;
        if (stat(full, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;
        if (!(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) continue;

        BotInfo *b = &reg->bots[reg->count];
        strncpy(b->name, entry->d_name, BOT_NAME_MAX - 1);
        b->name[BOT_NAME_MAX - 1] = '\0';
        strip_exe(b->name);

        strncpy(b->exe_path, full, BOT_PATH_MAX - 1);
        b->exe_path[BOT_PATH_MAX - 1] = '\0';

        reg->count++;
    }
    closedir(d);
#endif

    return reg;
}

/* ------------------------------------------------------------------
 * bot_registry_destroy
 * ------------------------------------------------------------------ */
void bot_registry_destroy(BotRegistry *reg)
{
    free(reg);
}

/* ------------------------------------------------------------------
 * bot_registry_find
 * ------------------------------------------------------------------ */
const BotInfo *bot_registry_find(const BotRegistry *reg, const char *name)
{
    if (!reg || !name) return NULL;
    int i;
    for (i = 0; i < reg->count; i++) {
#ifdef BOT_REG_WINDOWS
        if (_stricmp(reg->bots[i].name, name) == 0) return &reg->bots[i];
#else
        if (strcasecmp(reg->bots[i].name, name) == 0) return &reg->bots[i];
#endif
    }
    return NULL;
}
