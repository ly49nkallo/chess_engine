---
name: testing-chess-engine
description: Build, run, and test the raylib chess GUI and the UCI entrypoint end-to-end. Use when verifying move-generator, board-annotation (highlights/arrows), check-indicator, or UCI changes.
---

# Testing the chess_engine

## Build
Needs CMake + OpenGL/X11 dev libs (in the env blueprint). From repo root:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j
```
Binary: `build/CHESS_ENGINE/CHESS_ENGINE`.

## Unit tests (move generator + check logic)
Fast, no display needed. From `tests/`:
```
(cd tests && make && ./test.exe)
```
or `gcc test.c ../src/chess_engine.c ../src/utilities.c -o test.exe`. Exits non-zero on failure.
Good first signal for any `chess_engine.c` change.

## UCI entrypoint (shell, no GUI)
```
printf 'uci\nisready\nposition startpos moves e2e4 e7e5\ngo\nquit\n' | ./build/CHESS_ENGINE/CHESS_ENGINE uci 2>/dev/null
```
Expect `uciok`, `readyok`, then `bestmove <legal>`. stdout must be clean UCI only
(engine logs go to stderr). A `bestmove` that captures an own piece (e.g. `a1b1`) is a
move-generator bug.

## GUI testing (DISPLAY :0)
Launch: `cd build/CHESS_ENGINE && DISPLAY=:0 ./CHESS_ENGINE 2>/dev/null &`
(run from that dir so it finds `resources/`). It opens the GAME screen directly.
The window is a fixed 800x600 raylib window; wmctrl maximize has no useful effect.

Interactions (see `src/game_screen.c` game_screen_update):
- Left-click a piece = select (clears all annotations, shows green legal-move dots).
- Left-drag piece to a square = make the move.
- Right-click a tile = toggle a red highlight.
- Right-click + drag to another tile = toggle an orange arrow.
- Check indicator: red border drawn around the king of the side to move when in check.

### Caveat: raylib input polling vs synthetic clicks
The app polls `IsMouseButtonPressed/Released` once per frame. At the default 60fps a very
fast synthetic click (xdotool down+up within one ~16ms frame) can be MISSED entirely, so
right-clicks/arrows appear to do nothing. Workaround for automated testing: temporarily set
`frame_rate` in `src/main.c` to 240 (shorter frames -> reliably sampled), rebuild, and
REVERT before finishing. Real human clicks are long enough and don't need this.

### Right-drag (arrows) via xdotool
The computer tool has no right-drag primitive. Use xdotool on DISPLAY :0. Convert
1024x768 tool coords to the real 1600x1200 display with `*1600/1024` and `*1200/768`:
```
export DISPLAY=:0; conv(){ echo $(( $1*1600/1024 )) $(( $2*1200/768 )); }
xdotool mousemove $(conv X1 Y1) mousedown 3
xdotool mousemove $(conv X2 Y2); sleep 0.08
xdotool mouseup 3
```

### Board pixel mapping (in 1024x768 tool space, default window placement)
Files a-h x: 377,415,453,491,530,568,607,645. Ranks 1-8 y: 518,481,445,405,365,325,287,249.
Click the title bar (~511,182) first to focus the window. Re-screenshot if the window moves.

### A quick way to trigger a check (for the check indicator)
From the start position: 1. e2e4, 1... f7f5, 2. Qd1h5 — the queen checks the black king on
e8 along h5-g6-f7-e8. Verify each move with a screenshot; the queen's diagonal must stay
clear (don't let a knight land on f3 first).

## Devin Secrets Needed
None. All testing is local (no external services or credentials).
