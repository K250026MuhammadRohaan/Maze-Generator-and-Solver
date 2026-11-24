// kruskal.h - Module 2 (part 1): Kruskal maze generation using Union-Find.
#pragma once
#include "ds.h"
#include "maze.h"

// Room number of the room at (r, c), counting rooms left to right, top to bottom.
inline int roomNumber(int r, int c, int roomCols) {
    return (r / 2) * roomCols + c / 2;
}

// 1. Every room starts in its own group.
// 2. List all walls between two rooms and shuffle them (no sorting needed).
// 3. Go through the walls: open a wall only if the two rooms next to it are in
//    different groups (so a loop can never form), then join the groups.
// O(n * alpha(n)), which is almost O(n).
inline void generateKruskal(Maze& m) {
    m.fillWalls();
    int rows = m.rows(), cols = m.cols();
    int roomRows = (rows - 1) / 2, roomCols = (cols - 1) / 2;

    for (int i = 0; i < roomRows; ++i)
        for (int j = 0; j < roomCols; ++j) m.setOpen(2 * i + 1, 2 * j + 1);

    // A wall between two rooms has one odd and one even coordinate.
    int* walls = new int[rows * cols];       // cell numbers: row * cols + col
    int n = 0;
    for (int r = 1; r < rows - 1; ++r)
        for (int c = 1; c < cols - 1; ++c)
            if ((r + c) % 2 == 1) walls[n++] = r * cols + c;
    shuffleArray(walls, n);

    UnionFind groups(roomRows * roomCols);
    for (int k = 0; k < n; ++k) {
        int r = walls[k] / cols, c = walls[k] % cols;
        int roomA, roomB;
        if (r % 2 == 1) {     // wall with a room on its left and right
            roomA = roomNumber(r, c - 1, roomCols);
            roomB = roomNumber(r, c + 1, roomCols);
        } else {              // wall with a room above and below
            roomA = roomNumber(r - 1, c, roomCols);
            roomB = roomNumber(r + 1, c, roomCols);
        }
        if (groups.unite(roomA, roomB)) m.setOpen(r, c);
    }
    delete[] walls;
}
