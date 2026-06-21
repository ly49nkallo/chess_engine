/*
 * bot_player.c -- UCI subprocess client for the GUI game loop.
 *
 * Wraps a chess engine (or any UCI-compatible bot binary) as a child
 * process and exchanges UCI messages over anonymous pipes.  The polling
 * path is non-blocking so the 60-fps render loop is never stalled.
 *
 * Usage model:
 *   - bot_player_create(exe_path) spawns the binary with the "uci" arg.
 *   - bot_player_request_move() sends "position ... go movetime N".
 *   - bot_player_poll_move()    checks (non-blocking) for "bestmove".
 *
 * The exe_path is the path to the UCI-capable bot executable.  For the
 * built-in engine strategy this is CHESS_ENGINE itself; future bots may
 * be separate binaries that also speak UCI.
 */

#include "bot_player.h"
#include "utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#  define BP_WINDOWS 1
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  define BP_POSIX 1
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <fcntl.h>
#  include <errno.h>
#  include <signal.h>
#endif

#define BP_BUF (4096)

struct BotPlayer {
#ifdef BP_WINDOWS
    HANDLE proc;
    HANDLE thread;
    HANDLE pipe_read;
    HANDLE pipe_write;
#else
    pid_t  pid;
    int    fd_read;
    int    fd_write;
#endif
    char   buf[BP_BUF];
    int    buf_len;
    bool   thinking;
};

/* ------------------------------------------------------------------
 * Low-level pipe helpers
 * ------------------------------------------------------------------ */

static bool bp_write_all(BotPlayer *bp, const char *data, int len)
{
#ifdef BP_WINDOWS
    while (len > 0) {
        DWORD w = 0;
        if (!WriteFile(bp->pipe_write, data, (DWORD)len, &w, NULL) || w == 0)
            return false;
        data += w; len -= (int)w;
    }
    return true;
#else
    while (len > 0) {
        ssize_t w = write(bp->fd_write, data, (size_t)len);
        if (w <= 0) return false;
        data += w; len -= (int)w;
    }
    return true;
#endif
}

/* Non-blocking read. Returns bytes read, 0 if nothing pending, <0 on error. */
static int bp_read_nonblock(BotPlayer *bp, char *dst, int max)
{
#ifdef BP_WINDOWS
    DWORD avail = 0;
    if (!PeekNamedPipe(bp->pipe_read, NULL, 0, NULL, &avail, NULL)) return -1;
    if (avail == 0) return 0;
    DWORD r = 0;
    DWORD to_read = (avail < (DWORD)max) ? avail : (DWORD)max;
    if (!ReadFile(bp->pipe_read, dst, to_read, &r, NULL)) return -1;
    return (int)r;
#else
    int flags = fcntl(bp->fd_read, F_GETFL, 0);
    fcntl(bp->fd_read, F_SETFL, flags | O_NONBLOCK);
    ssize_t r = read(bp->fd_read, dst, (size_t)max);
    fcntl(bp->fd_read, F_SETFL, flags);
    if (r < 0 && errno == EAGAIN) return 0;
    return (int)r;
#endif
}

/* Blocking read of one line. Used only during handshake. */
static bool bp_readline_blocking(BotPlayer *bp, char *out, int max)
{
    for (;;) {
        for (int i = 0; i < bp->buf_len; i++) {
            if (bp->buf[i] == '\n') {
                int len = i;
                if (len > 0 && bp->buf[len-1] == '\r') len--;
                if (len >= max) len = max - 1;
                memcpy(out, bp->buf, (size_t)len);
                out[len] = '\0';
                int rem = bp->buf_len - i - 1;
                memmove(bp->buf, bp->buf + i + 1, (size_t)rem);
                bp->buf_len = rem;
                return true;
            }
        }
        int space = BP_BUF - bp->buf_len - 1;
        if (space <= 0) { bp->buf_len = 0; space = BP_BUF - 1; }
#ifdef BP_WINDOWS
        DWORD r = 0;
        if (!ReadFile(bp->pipe_read, bp->buf + bp->buf_len, (DWORD)space, &r, NULL))
            return false;
        bp->buf_len += (int)r;
#else
        ssize_t r = read(bp->fd_read, bp->buf + bp->buf_len, (size_t)space);
        if (r <= 0) return false;
        bp->buf_len += (int)r;
#endif
    }
}

static void bp_send(BotPlayer *bp, const char *cmd)
{
    bp_write_all(bp, cmd, (int)strlen(cmd));
    bp_write_all(bp, "\n", 1);
}

/* ------------------------------------------------------------------
 * bot_player_create
 *
 * Spawns exe_path as a UCI engine (passes "uci" as argv[1]).
 * Performs the uci/uciok and isready/readyok handshake.
 * ------------------------------------------------------------------ */
BotPlayer *bot_player_create(const char *exe_path, const char *engine_path)
{


    BotPlayer *bp = (BotPlayer *)calloc(1, sizeof(BotPlayer));
    if (!bp) return NULL;

#ifdef BP_WINDOWS
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE stdin_rd, stdin_wr, stdout_rd, stdout_wr;

    if (!CreatePipe(&stdin_rd,  &stdin_wr,  &sa, 0) ||
        !CreatePipe(&stdout_rd, &stdout_wr, &sa, 0)) {
        fprintf(stderr, "[bot_player] CreatePipe failed\n");
        free(bp); return NULL;
    }
    SetHandleInformation(stdin_wr,  HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stdout_rd, HANDLE_FLAG_INHERIT, 0);

    char cmdline[1024];
    if (engine_path && engine_path[0])
        snprintf(cmdline, sizeof(cmdline), "\"%s\" uci \"%s\"", exe_path, engine_path);
    else
        snprintf(cmdline, sizeof(cmdline), "\"%s\" uci", exe_path);

    STARTUPINFOA si; PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si)); ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.hStdInput  = stdin_rd;
    si.hStdOutput = stdout_wr;
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags   |= STARTF_USESTDHANDLES;

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "[bot_player] CreateProcess failed for: %s\n", cmdline);
        CloseHandle(stdin_rd); CloseHandle(stdin_wr);
        CloseHandle(stdout_rd); CloseHandle(stdout_wr);
        free(bp); return NULL;
    }
    CloseHandle(stdin_rd);
    CloseHandle(stdout_wr);

    bp->proc       = pi.hProcess;
    bp->thread     = pi.hThread;
    bp->pipe_read  = stdout_rd;
    bp->pipe_write = stdin_wr;

#else /* POSIX */
    int to_child[2], from_child[2];
    if (pipe(to_child) < 0 || pipe(from_child) < 0) {
        perror("[bot_player] pipe"); free(bp); return NULL;
    }
    pid_t pid = fork();
    if (pid < 0) { perror("[bot_player] fork"); free(bp); return NULL; }
    if (pid == 0) {
        dup2(to_child[0],   STDIN_FILENO);
        dup2(from_child[1], STDOUT_FILENO);
        close(to_child[0]); close(to_child[1]);
        close(from_child[0]); close(from_child[1]);
        if (engine_path && engine_path[0])
            execl(exe_path, exe_path, "uci", engine_path, (char *)NULL);
        else
            execl(exe_path, exe_path, "uci", (char *)NULL);
        perror("[bot_player] execl"); _exit(1);
    }
    close(to_child[0]); close(from_child[1]);
    bp->pid      = pid;
    bp->fd_read  = from_child[0];
    bp->fd_write = to_child[1];
#endif

    /* UCI handshake */
    bp_send(bp, "uci");
    {
        char line[256];
        while (bp_readline_blocking(bp, line, sizeof(line)))
            if (strncmp(line, "uciok", 5) == 0) break;
    }
    bp_send(bp, "isready");
    {
        char line[256];
        while (bp_readline_blocking(bp, line, sizeof(line)))
            if (strncmp(line, "readyok", 7) == 0) break;
    }

    fprintf(stderr, "[bot_player] Ready: %s\n", exe_path);
    return bp;
}

/* ------------------------------------------------------------------
 * bot_player_destroy
 * ------------------------------------------------------------------ */
void bot_player_destroy(BotPlayer *bp)
{
    if (!bp) return;
    bp_send(bp, "quit");
#ifdef BP_WINDOWS
    WaitForSingleObject(bp->proc, 2000);
    CloseHandle(bp->proc); CloseHandle(bp->thread);
    CloseHandle(bp->pipe_read); CloseHandle(bp->pipe_write);
#else
    close(bp->fd_write); close(bp->fd_read);
    waitpid(bp->pid, NULL, 0);
#endif
    free(bp);
}

/* ------------------------------------------------------------------
 * bot_player_new_game
 * ------------------------------------------------------------------ */
void bot_player_new_game(BotPlayer *bp)
{
    if (!bp) return;
    bp_send(bp, "ucinewgame");
    bp->thinking = false;
    bp->buf_len  = 0;
}

/* ------------------------------------------------------------------
 * bot_player_request_move
 * ------------------------------------------------------------------ */
void bot_player_request_move(BotPlayer *bp,
                              const char *move_history,
                              const char *start_fen,
                              int movetime_ms)
{
    if (!bp || bp->thinking) return;

    char pos_cmd[2048];
    if (start_fen && start_fen[0]) {
        snprintf(pos_cmd, sizeof(pos_cmd), "position fen %s", start_fen);
    } else {
        snprintf(pos_cmd, sizeof(pos_cmd), "position startpos");
    }
    if (move_history && move_history[0]) {
        strncat(pos_cmd, " moves ", sizeof(pos_cmd) - strlen(pos_cmd) - 1);
        strncat(pos_cmd, move_history,
                sizeof(pos_cmd) - strlen(pos_cmd) - 1);
    }
    bp_send(bp, pos_cmd);

    char go_cmd[64];
    snprintf(go_cmd, sizeof(go_cmd), "go movetime %d", movetime_ms);
    bp_send(bp, go_cmd);

    bp->thinking = true;
}

/* ------------------------------------------------------------------
 * bot_player_poll_move
 * ------------------------------------------------------------------ */
bool bot_player_poll_move(BotPlayer *bp, char *move_out)
{
    if (!bp || !bp->thinking) return false;

    /* Drain available bytes */
    int space = BP_BUF - bp->buf_len - 1;
    if (space > 0) {
        int got = bp_read_nonblock(bp, bp->buf + bp->buf_len, space);
        if (got > 0) bp->buf_len += got;
    }

    /* Scan for "bestmove" line */
    char *p = bp->buf;
    int remaining = bp->buf_len;
    while (remaining > 0) {
        char *nl = (char *)memchr(p, '\n', (size_t)remaining);
        if (!nl) break;

        int line_len = (int)(nl - p);
        if (line_len > 0 && p[line_len - 1] == '\r') line_len--;

        if (line_len >= 8 && strncmp(p, "bestmove", 8) == 0) {
            const char *m = p + 8;
            while (*m == ' ') m++;
            int i = 0;
            while (*m && *m != ' ' && *m != '\r' && *m != '\n'
                   && i < BP_MOVE_MAX - 1)
                move_out[i++] = *m++;
            move_out[i] = '\0';

            int consumed = (int)(nl - bp->buf) + 1;
            memmove(bp->buf, bp->buf + consumed,
                    (size_t)(bp->buf_len - consumed));
            bp->buf_len -= consumed;
            bp->thinking  = false;
            return true;
        }

        int skip = (int)(nl - bp->buf) + 1;
        memmove(bp->buf, bp->buf + skip, (size_t)(bp->buf_len - skip));
        bp->buf_len -= skip;
        p = bp->buf;
        remaining = bp->buf_len;
    }
    return false;
}

/* ------------------------------------------------------------------
 * bot_player_is_thinking
 * ------------------------------------------------------------------ */
bool bot_player_is_thinking(BotPlayer *bp)
{
    return bp && bp->thinking;
}
