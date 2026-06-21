/*
 * uci_client.c -- spawn the chess engine as a subprocess and talk UCI.
 *
 * Uses Win32 CreateProcess + anonymous pipes on Windows,
 * or fork/execvp + POSIX pipes everywhere else.
 */

#include "bot.h"

#if defined(_WIN32) || defined(_WIN64)
#  define BOT_WINDOWS 1
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  define BOT_POSIX 1
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <signal.h>
#endif

/* ------------------------------------------------------------------
 * Platform-specific subprocess handle
 * ------------------------------------------------------------------ */
struct UciClient {
#ifdef BOT_WINDOWS
    HANDLE proc;            /* engine process handle */
    HANDLE thread;          /* engine main thread handle */
    HANDLE pipe_read;       /* read end  (bot reads engine stdout) */
    HANDLE pipe_write;      /* write end (bot writes engine stdin)  */
#else
    pid_t  pid;
    int    fd_read;         /* read end  (bot reads engine stdout) */
    int    fd_write;        /* write end (bot writes engine stdin)  */
#endif
    /* Leftover bytes from a partial readline */
    char   buf[UCI_LINE_MAX * 2];
    int    buf_len;
};

/* ------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------ */

/* Write exactly 'len' bytes to the engine's stdin pipe. */
static bool pipe_write_all(UciClient *c, const char *data, int len)
{
#ifdef BOT_WINDOWS
    DWORD written = 0;
    while (len > 0) {
        DWORD w = 0;
        if (!WriteFile(c->pipe_write, data, (DWORD)len, &w, NULL) || w == 0)
            return false;
        data += w;
        len  -= (int)w;
        written += w;
    }
    return true;
#else
    while (len > 0) {
        ssize_t w = write(c->fd_write, data, (size_t)len);
        if (w <= 0) return false;
        data += w;
        len  -= (int)w;
    }
    return true;
#endif
}

/* Read up to 'max' bytes from the engine's stdout pipe.
 * Returns number of bytes read, or <= 0 on error/EOF. */
static int pipe_read_some(UciClient *c, char *dst, int max)
{
#ifdef BOT_WINDOWS
    DWORD r = 0;
    if (!ReadFile(c->pipe_read, dst, (DWORD)max, &r, NULL)) return -1;
    return (int)r;
#else
    ssize_t r = read(c->fd_read, dst, (size_t)max);
    return (int)r;
#endif
}

/* ------------------------------------------------------------------
 * uci_client_create
 * ------------------------------------------------------------------ */
UciClient *uci_client_create(const char *engine_path)
{
    UciClient *c = (UciClient *)calloc(1, sizeof(UciClient));
    if (!c) return NULL;

#ifdef BOT_WINDOWS
    /* Create two anonymous pipes: one for stdin, one for stdout. */
    SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle       = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE stdin_rd,  stdin_wr;
    HANDLE stdout_rd, stdout_wr;

    if (!CreatePipe(&stdin_rd,  &stdin_wr,  &sa, 0) ||
        !CreatePipe(&stdout_rd, &stdout_wr, &sa, 0)) {
        fprintf(stderr, "[bot] CreatePipe failed (%lu)\n", GetLastError());
        free(c);
        return NULL;
    }
    /* The bot's end of each pipe must NOT be inherited by the child. */
    SetHandleInformation(stdin_wr,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stdout_rd, HANDLE_FLAG_INHERIT, 0);

    /* Build the command line: "<engine_path> uci" */
    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "\"%s\" uci", engine_path);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb          = sizeof(si);
    si.hStdInput   = stdin_rd;
    si.hStdOutput  = stdout_wr;
    si.hStdError   = GetStdHandle(STD_ERROR_HANDLE); /* keep engine stderr on console */
    si.dwFlags    |= STARTF_USESTDHANDLES;

    if (!CreateProcessA(
            NULL, cmdline,
            NULL, NULL,
            TRUE,           /* inherit handles */
            0,
            NULL, NULL,
            &si, &pi)) {
        fprintf(stderr, "[bot] CreateProcess failed (%lu) for: %s\n",
                GetLastError(), cmdline);
        CloseHandle(stdin_rd); CloseHandle(stdin_wr);
        CloseHandle(stdout_rd); CloseHandle(stdout_wr);
        free(c);
        return NULL;
    }

    /* Close child-side handles in the parent; keep bot-side handles. */
    CloseHandle(stdin_rd);
    CloseHandle(stdout_wr);

    c->proc       = pi.hProcess;
    c->thread     = pi.hThread;
    c->pipe_read  = stdout_rd;
    c->pipe_write = stdin_wr;

#else /* POSIX */
    int to_child[2];   /* bot writes [1], child reads [0] */
    int from_child[2]; /* child writes [1], bot reads [0] */

    if (pipe(to_child) < 0 || pipe(from_child) < 0) {
        perror("[bot] pipe");
        free(c);
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("[bot] fork");
        free(c);
        return NULL;
    }
    if (pid == 0) {
        /* Child: redirect stdio and exec the engine */
        dup2(to_child[0],   STDIN_FILENO);
        dup2(from_child[1], STDOUT_FILENO);
        dup2(from_child[1], STDERR_FILENO);
        close(to_child[0]); close(to_child[1]);
        close(from_child[0]); close(from_child[1]);
        execl(engine_path, engine_path, "uci", (char *)NULL);
        perror("[bot] execl");
        _exit(1);
    }

    /* Parent: keep our ends, close child-side ends */
    close(to_child[0]);
    close(from_child[1]);

    c->pid      = pid;
    c->fd_read  = from_child[0];
    c->fd_write = to_child[1];
#endif

    c->buf_len = 0;

    /* --- UCI handshake --- */
    /* 1. Send "uci" and wait for "uciok" */
    uci_client_send(c, "uci");
    {
        char line[UCI_LINE_MAX];
        while (uci_client_readline(c, line)) {
            fprintf(stderr, "[engine] %s\n", line);
            if (strncmp(line, "uciok", 5) == 0) break;
        }
    }
    /* 2. Send "isready" and wait for "readyok" */
    uci_client_send(c, "isready");
    {
        char line[UCI_LINE_MAX];
        while (uci_client_readline(c, line)) {
            fprintf(stderr, "[engine] %s\n", line);
            if (strncmp(line, "readyok", 7) == 0) break;
        }
    }

    return c;
}

/* ------------------------------------------------------------------
 * uci_client_destroy
 * ------------------------------------------------------------------ */
void uci_client_destroy(UciClient *client)
{
    if (!client) return;
    uci_client_send(client, "quit");
#ifdef BOT_WINDOWS
    WaitForSingleObject(client->proc, 2000);
    CloseHandle(client->proc);
    CloseHandle(client->thread);
    CloseHandle(client->pipe_read);
    CloseHandle(client->pipe_write);
#else
    close(client->fd_write);
    close(client->fd_read);
    waitpid(client->pid, NULL, 0);
#endif
    free(client);
}

/* ------------------------------------------------------------------
 * uci_client_send
 * ------------------------------------------------------------------ */
void uci_client_send(UciClient *client, const char *cmd)
{
    if (!client || !cmd) return;
    fprintf(stderr, "[bot->engine] %s\n", cmd);
    pipe_write_all(client, cmd, (int)strlen(cmd));
    pipe_write_all(client, "\n", 1);
}

/* ------------------------------------------------------------------
 * uci_client_readline
 * Read bytes until a '\n', return trimmed line (no CR/LF).
 * ------------------------------------------------------------------ */
bool uci_client_readline(UciClient *client, char *out)
{
    if (!client || !out) return false;

    for (;;) {
        /* Scan the buffer for a newline */
        for (int i = 0; i < client->buf_len; i++) {
            if (client->buf[i] == '\n') {
                /* Copy line (without newline) to out */
                int len = i;
                if (len > 0 && client->buf[len - 1] == '\r') len--;
                if (len >= UCI_LINE_MAX) len = UCI_LINE_MAX - 1;
                memcpy(out, client->buf, len);
                out[len] = '\0';
                /* Shift remaining bytes to front */
                int remaining = client->buf_len - i - 1;
                memmove(client->buf, client->buf + i + 1, (size_t)remaining);
                client->buf_len = remaining;
                return true;
            }
        }
        /* Need more data -- read from pipe */
        int space = (int)sizeof(client->buf) - client->buf_len - 1;
        if (space <= 0) {
            /* Buffer full with no newline; flush and try again */
            client->buf_len = 0;
            space = (int)sizeof(client->buf) - 1;
        }
        int n = pipe_read_some(client,
                               client->buf + client->buf_len,
                               space);
        if (n <= 0) return false; /* EOF or error */
        client->buf_len += n;
    }
}

/* ------------------------------------------------------------------
 * uci_client_send_position
 * ------------------------------------------------------------------ */
void uci_client_send_position(UciClient *client, const BotState *state)
{
    char cmd[UCI_LINE_MAX];
    int  pos = 0;

    if (state->start_fen) {
        pos += snprintf(cmd + pos, sizeof(cmd) - (size_t)pos,
                        "position fen %s", state->start_fen);
    } else {
        pos += snprintf(cmd + pos, sizeof(cmd) - (size_t)pos,
                        "position startpos");
    }

    if (state->move_count > 0) {
        pos += snprintf(cmd + pos, sizeof(cmd) - (size_t)pos, " moves");
        for (int i = 0; i < state->move_count; i++) {
            pos += snprintf(cmd + pos, sizeof(cmd) - (size_t)pos,
                            " %s", state->moves[i]);
        }
    }

    uci_client_send(client, cmd);
}

/* ------------------------------------------------------------------
 * uci_client_go_get_bestmove
 * ------------------------------------------------------------------ */
bool uci_client_go_get_bestmove(UciClient *client, int movetime_ms,
                                 char *move_out)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "go movetime %d", movetime_ms);
    uci_client_send(client, cmd);

    char line[UCI_LINE_MAX];
    while (uci_client_readline(client, line)) {
        fprintf(stderr, "[engine] %s\n", line);
        if (strncmp(line, "bestmove", 8) == 0) {
            /* Format: "bestmove <move> [ponder <move>]" */
            const char *p = line + 8;
            while (*p == ' ') p++;
            int i = 0;
            while (*p && *p != ' ' && i < 7) move_out[i++] = *p++;
            move_out[i] = '\0';
            /* "0000" means no legal move (checkmate / stalemate) */
            return strcmp(move_out, "0000") != 0;
        }
    }
    return false;
}
