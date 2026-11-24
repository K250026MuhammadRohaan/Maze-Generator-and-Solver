# Maze Generator and Solver (Data Structures, BAI-3A)

Console C++17 project following the proposal. Header-only modules, one `main.cpp`.

## Build and run
    g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o maze      (Windows: build.bat, then maze.exe)

## Files / modules
| File | Module | Contents |
|---|---|---|
| `ds.h` | shared | Custom `Stack`, `Queue` (circular, growing), `UnionFind` (path compression + union by rank) |
| `maze.h` | 1 (Sarim) | `Maze` grid, bounds/collision checks, console display, DFS backtracking generator, `addLoops`, `tryMove` |
| `game.h` | 1 (Sarim) | WASD + Enter gameplay, elapsed time, move count, score |
| `kruskal.h` | 2 (Hussain) | Kruskal generation with Union-Find (shuffled walls, no sort) |
| `leaderboard.h` | 2 (Hussain) | `scores.txt` load/save, top five, insertion sort, merge sort, linear + binary search |
| `solvers.h` | 3 (Rohaan) | BFS (custom queue), iterative DFS (custom stack), visited/parent arrays, path reconstruction |
| `compare.h` | 3 (Rohaan) | Generate-and-solve demo, generator and solver comparison (time, path length, explored cells) |
| `tests.h` | 3 (Rohaan) | Normal and edge-case tests (menu option 5) |
| `ui.h` | shared | Input helpers, `MazeConfig`, generator selection |

## Design notes
- Grid is odd-sized (Easy 11x11, Medium 21x21, Hard 31x31; custom 5-101). Rooms sit on odd/odd cells, walls between them are opened by the generators. Start = top-left room, exit = bottom-right room.
- Algorithms use only the custom stack/queue/Union-Find (no STL containers); `std::unique_ptr` and raw arrays are used for plain storage.
- Score = 1000 x difficulty - 2 x seconds - 5 x (moves beyond the BFS shortest path), never below 0.
- Leaderboard format (`scores.txt`): `name score moves seconds`, one per line. Malformed lines are skipped; top five are kept.
