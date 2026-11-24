// maze.h - Module 1: the maze grid, DFS generation, extra loops,
//          console display and player movement.
#pragma once
#include <stdio.h>
#include "ds.h"
#include "utils.h"

struct Cell { int r, c; };      // a position: row and column

// The four directions: up, right, down, left (used by generators and solvers).
const int DIR_R[4] = {-1, 0, 1, 0};
const int DIR_C[4] = {0, 1, 0, -1};

// ----------------------------------------------------------------- Maze ----
// A grid of rows x cols (both odd). Rooms sit on odd/odd positions, and the
// cell between two neighbouring rooms is a wall that can be knocked down:
//
//   # # # # #
//   # R . R #      R = room, . = passage (a wall that was opened)
//   # . # . #      # = wall
//   # R . R #
//   # # # # #
class Maze {
    int rows_, cols_;
    char* grid_;     // 1 = wall, 0 = open

    Maze(const Maze&);               // a Maze cannot be copied
    Maze& operator=(const Maze&);

public:
    static bool validSize(int r, int c) {
        return r >= 5 && c >= 5 && r <= 101 && c <= 101 && r % 2 == 1 && c % 2 == 1;
    }

    // Callers should check validSize first; a bad size gives a safe 5x5 maze.
    Maze(int r, int c) : rows_(r), cols_(c), grid_(NULL) {
        if (!validSize(r, c)) { rows_ = 5; cols_ = 5; }
        grid_ = new char[rows_ * cols_];
        fillWalls();
    }
    ~Maze() { delete[] grid_; }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int rooms() const { return ((rows_ - 1) / 2) * ((cols_ - 1) / 2); }
    Cell start() const { Cell s = {1, 1}; return s; }                    // top-left room
    Cell goal() const { Cell g = {rows_ - 2, cols_ - 2}; return g; }     // bottom-right room

    bool inBounds(int r, int c) const { return r >= 0 && r < rows_ && c >= 0 && c < cols_; }
    bool isWall(int r, int c) const { return !inBounds(r, c) || grid_[r * cols_ + c] == 1; }
    bool isOpen(int r, int c) const { return !isWall(r, c); }
    void setOpen(int r, int c) { if (inBounds(r, c)) grid_[r * cols_ + c] = 0; }
    void fillWalls() {
        for (int i = 0; i < rows_ * cols_; ++i) grid_[i] = 1;
    }

    int openCount() const {
        int n = 0;
        for (int i = 0; i < rows_ * cols_; ++i)
            if (grid_[i] == 0) ++n;
        return n;
    }

    // Number of opened passages between rooms. A perfect maze has exactly rooms() - 1.
    int openConnectors() const {
        int n = 0;
        for (int r = 1; r < rows_ - 1; ++r)
            for (int c = 1; c < cols_ - 1; ++c)
                if ((r + c) % 2 == 1 && grid_[r * cols_ + c] == 0) ++n;
        return n;
    }

    // Prints the maze. player: where to draw P (or NULL).
    // mark: array of rows*cols chars, non-zero = draw a path dot (or NULL).
    void display(const Cell* player = NULL, const char* mark = NULL) const {
        Cell g = goal();
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) {
                if (player != NULL && player->r == r && player->c == c) printf("P ");
                else if (r == 1 && c == 1) printf("S ");
                else if (r == g.r && c == g.c) printf("E ");
                else if (isWall(r, c)) printf("##");
                else if (mark != NULL && mark[r * cols_ + c]) printf(". ");
                else printf("  ");
            }
            printf("\n");
        }
    }
};

// ------------------------------------------------- DFS (backtracking) gen ----
// Start in a room, keep moving to a random unvisited neighbouring room
// (knocking down the wall between them). When stuck, go back (pop the stack)
// until a room with an unvisited neighbour is found. O(n).
// Stack items are cell numbers: row * cols + col.
inline void generateDFS(Maze& m) {
    m.fillWalls();
    int cols = m.cols();
    Stack path(64);

    Cell s = m.start();
    m.setOpen(s.r, s.c);
    path.push(s.r * cols + s.c);

    while (!path.empty()) {
        int cur = path.top();
        int r = cur / cols, c = cur % cols;

        // Which directions lead to a room that is still a wall (not visited)?
        int options[4], count = 0;
        for (int d = 0; d < 4; ++d) {
            int nr = r + 2 * DIR_R[d], nc = c + 2 * DIR_C[d];    // the room 2 steps away
            if (m.inBounds(nr, nc) && m.isWall(nr, nc)) options[count++] = d;
        }

        if (count == 0) {                 // dead end: go back
            path.pop();
            continue;
        }

        int d = options[randInt(count)];
        int nr = r + 2 * DIR_R[d], nc = c + 2 * DIR_C[d];
        m.setOpen(r + DIR_R[d], c + DIR_C[d]);   // knock down the wall in between
        m.setOpen(nr, nc);                       // open the new room
        path.push(nr * cols + nc);
    }
}

// Opens up to `count` extra walls that sit between two open rooms. This
// creates loops (more than one route). Returns how many were opened. O(n).
inline int addLoops(Maze& m, int count) {
    if (count <= 0) return 0;
    int cols = m.cols();
    int* candidates = new int[m.rows() * cols];     // cell numbers of walls we may open
    int k = 0;
    for (int r = 1; r < m.rows() - 1; ++r) {
        for (int c = 1; c < cols - 1; ++c) {
            if (m.isOpen(r, c)) continue;
            bool sideways = (r % 2 == 1) && (c % 2 == 0) && m.isOpen(r, c - 1) && m.isOpen(r, c + 1);
            bool vertical = (r % 2 == 0) && (c % 2 == 1) && m.isOpen(r - 1, c) && m.isOpen(r + 1, c);
            if (sideways || vertical) candidates[k++] = r * cols + c;
        }
    }
    shuffleArray(candidates, k);
    int opened = count < k ? count : k;
    for (int i = 0; i < opened; ++i) m.setOpen(candidates[i] / cols, candidates[i] % cols);
    delete[] candidates;
    return opened;
}

// ----------------------------------------------------------- movement ----
// key: W/A/S/D (upper or lower case). Returns true only if the player moved.
inline bool tryMove(const Maze& m, Cell& p, char key) {
    int dr = 0, dc = 0;
    switch (toUpperChar(key)) {
        case 'W': dr = -1; break;
        case 'S': dr = 1; break;
        case 'A': dc = -1; break;
        case 'D': dc = 1; break;
        default: return false;
    }
    if (m.isWall(p.r + dr, p.c + dc)) return false;     // wall (or outside the grid)
    p.r += dr;
    p.c += dc;
    return true;
}
