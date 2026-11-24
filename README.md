# Maze Generator and Solver (Data Structures, BAI-3A)

Console C++ project. Header-only modules, one `main.cpp`.
No STL and no `std`: only the plain C headers `stdio.h`, `stdlib.h` and `time.h` are used.
Every data structure and algorithm is written by us.

## Build and run
    g++ -O2 -Wall -Wextra main.cpp -o maze      (Windows: build.bat, then maze.exe)

## Files / modules
| File | Module | Contents |
|---|---|---|
| `utils.h` | shared | Random numbers, shuffle, char-array text helpers, keyboard input (`readLine`, `readInt`, `readYesNo`) |
| `ds.h` | shared | Our own `Stack` and `Queue` (circular, growing, store ints) and `UnionFind` (path compression + union by rank) |
| `maze.h` | 1 (Sarim) | `Maze` grid, bounds/collision checks, console display, DFS backtracking generator, `addLoops`, `tryMove` |
| `game.h` | 1 (Sarim) | WASD + Enter gameplay, elapsed time, move count, score |
| `kruskal.h` | 2 (Hussain) | Kruskal generation with Union-Find (shuffled walls, no sort) |
| `leaderboard.h` | 2 (Hussain) | `scores.txt` load/save, top five, insertion sort, merge sort, linear + binary search |
| `solvers.h` | 3 (Rohaan) | BFS (our queue), iterative DFS (our stack), parent array, path reconstruction |
| `compare.h` | 3 (Rohaan) | Generate-and-solve demo, generator and solver comparison (time, path length, explored cells) |
| `tests.h` | 3 (Rohaan) | Normal and edge-case tests (menu option 5) |
| `ui.h` | shared | `MazeConfig`, generator selection, difficulty menu |

## Design notes
- Grid is odd-sized (Easy 11x11, Medium 21x21, Hard 31x31; custom 5-101). Rooms sit on odd/odd cells, walls between them are opened by the generators. Start = top-left room, goal = bottom-right room.
- A cell is often stored as one number: `cell = row * cols + col`. That is why `Stack` and `Queue` only need to store ints.
- No exceptions: an empty `Stack`/`Queue` returns -1 from `pop()`, and a bad maze size is checked with `Maze::validSize()` before a `Maze` is made.
- Plain `new[]` / `delete[]` arrays are used for storage (each class deletes its own array in its destructor).
- Score = 1000 x difficulty - 2 x seconds - 5 x (moves beyond the BFS shortest path), never below 0.
- Leaderboard format (`scores.txt`): `name score moves seconds`, one per line. Malformed lines are skipped; top five are kept.
- Timing uses `clock()`. Each run is repeated many times (see `GEN_REPS` / `SOLVE_REPS` in `compare.h`) and averaged, because one run is too fast to measure.
