// kruskal.h - Module 2 (part 1): Kruskal maze generation using Union-Find.
#pragma once
#include <memory>
#include "ds.h"
#include "maze.h"

struct WallEdge { int r, c; int roomA, roomB; };

// Every room starts in its own set. Walls are shuffled (no sorting needed);
// a wall is opened only if it joins two different sets. O(n * alpha(n)).
inline void generateKruskal(Maze& m) {
    m.fillWalls();
    int rr = (m.rows() - 1) / 2, rc = (m.cols() - 1) / 2, rooms = rr * rc;
    for (int i = 0; i < rr; ++i)
        for (int j = 0; j < rc; ++j) m.setOpen(2 * i + 1, 2 * j + 1);

    std::unique_ptr<WallEdge[]> walls(new WallEdge[2 * rooms]);
    int n = 0;
    for (int i = 0; i < rr; ++i) {
        for (int j = 0; j < rc; ++j) {
            int id = i * rc + j;
            if (j + 1 < rc) walls[n++] = WallEdge{2 * i + 1, 2 * j + 2, id, id + 1};
            if (i + 1 < rr) walls[n++] = WallEdge{2 * i + 2, 2 * j + 1, id, id + rc};
        }
    }
    shuffleArray(walls.get(), n);

    UnionFind uf(rooms);
    for (int k = 0; k < n; ++k)
        if (uf.unite(walls[k].roomA, walls[k].roomB)) m.setOpen(walls[k].r, walls[k].c);
}
