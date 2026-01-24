# TODO

- [x] Debug the bbroken move generator logic using the program `print_bitboard.exe` at `~/source` and Cmake Debugger (command pallet)

## chess_engine.c

- [ ] Implement allowed moves generator (both bitboard and list)
- [x] Allowed move generator logic bugged for pawns. Double push and H-file pawn broken.
- [ ] Write script to test moves generator
- [ ] Refactor king check checking

## game_screen.c

- [x] Select tile by left clicking. Draw small circles on tiles that can be moved to
- [ ] Highlight tile by right clicking
- [ ] Draw Arrow by right clicking and dragging (all arrows deleted when piece is selected)

## uci.c

- [ ] Make a program client that can recieve UCI data and send UCI data. Just random moves is fine.  Have main.c interface with chess_engine.c via this UCI connection. 
  - [ ] 