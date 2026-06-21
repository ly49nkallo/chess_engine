#ifndef CE_BOT_REGISTRY_H
#define CE_BOT_REGISTRY_H

/*
 * bot_registry.h -- discovery and listing of available chess bots.
 *
 * Convention:
 *   Built bot executables are copied into a "bots/" directory that lives
 *   alongside the CHESS_ENGINE binary.  Each executable's filename (minus
 *   any ".exe" extension) is treated as the bot's display name.
 *
 *   Example layout after a build:
 *     build/CHESS_ENGINE/CHESS_ENGINE.exe
 *     build/CHESS_ENGINE/bots/bogo-bot.exe
 *
 * Usage:
 *   BotRegistry *reg = bot_registry_create();
 *   for (int i = 0; i < reg->count; i++)
 *       printf("%s -> %s\n", reg->bots[i].name, reg->bots[i].exe_path);
 *   bot_registry_destroy(reg);
 */

#include <stdbool.h>

#define BOT_NAME_MAX  64
#define BOT_PATH_MAX 512
#define BOT_REGISTRY_MAX 32

typedef struct BotInfo {
    char name[BOT_NAME_MAX];      /* display name (e.g. "bogo-bot") */
    char exe_path[BOT_PATH_MAX];  /* absolute/relative path to executable */
} BotInfo;

typedef struct BotRegistry {
    BotInfo bots[BOT_REGISTRY_MAX];
    int     count;
} BotRegistry;

/*
 * bot_registry_create -- allocate and populate the registry by scanning
 * the "bots/" directory next to the engine executable.
 *
 * If the bots/ directory does not exist or is empty, returns a registry
 * with count == 0 (not an error).
 */
BotRegistry *bot_registry_create(void);

/*
 * bot_registry_destroy -- free the registry.
 */
void bot_registry_destroy(BotRegistry *reg);

/*
 * bot_registry_find -- look up a bot by name (case-insensitive).
 * Returns a pointer into reg->bots, or NULL if not found.
 */
const BotInfo *bot_registry_find(const BotRegistry *reg, const char *name);

/*
 * get_exe_path -- fill buf with the full path of the running executable.
 * Falls back to "." on failure.  size is the buffer capacity in bytes.
 */
void get_exe_path(char *buf, int size);

#endif /* CE_BOT_REGISTRY_H */
