// solvers.h - Module 3: BFS (uses our Queue) and DFS (uses our Stack) solvers.
// Both find a path from the start to the goal and remember the way back with
// a parent[] array: parent[cell] = the cell we came from.
#pragma once
#include <time.h>
#include "ds.h"
#include "maze.h"

const int NOT_VISITED = -2;     // value in parent[] for a cell we have not reached yet
const int NO_PARENT = -1;       // value in parent[] for the start cell

struct SolveResult {
    bool found;
    int pathLength;     // steps from start to goal
    int pathCells;      // cells on the path (pathLength + 1)
    int explored;       // cells taken off the queue/stack and processed
    double micros;      // average run time in microseconds
    Cell* path;         // start ... goal (NULL if not found)

    SolveResult() : found(false), pathLength(0), pathCells(0), explored(0), micros(0), path(NULL) {}
    ~SolveResult() { delete[] path; }

    void clear() {      // forget the previous answer
        found = false;
        pathLength = 0;
        pathCells = 0;
        explored = 0;
        delete[] path;
        path = NULL;
    }

private:
    SolveResult(const SolveResult&);
    SolveResult& operator=(const SolveResult&);
};

// Follows parent[] from the goal back to the start, then stores the path
// in the right order (start first).
inline void buildPath(const Maze& m, const int* parent, SolveResult& result) {
    int cols = m.cols();
    Cell g = m.goal();
    int goalIdx = g.r * cols + g.c;

    int len = 0;
    for (int cur = goalIdx; cur != NO_PARENT; cur = parent[cur]) ++len;

    result.path = new Cell[len];
    result.pathCells = len;
    result.pathLength = len - 1;
    int i = len - 1;                     // fill from the back
    for (int cur = goalIdx; cur != NO_PARENT; cur = parent[cur]) {
        result.path[i].r = cur / cols;
        result.path[i].c = cur % cols;
        --i;
    }
}

// BFS: visits cells in rings of equal distance from the start, so the first
// time it reaches the goal is along a shortest path.
inline void bfsOnce(const Maze& m, SolveResult& result) {
    result.clear();
    Cell s = m.start(), g = m.goal();
    if (m.isWall(s.r, s.c) || m.isWall(g.r, g.c)) return;

    int cols = m.cols(), total = m.rows() * cols;
    int startIdx = s.r * cols + s.c, goalIdx = g.r * cols + g.c;
    int* parent = new int[total];
    for (int i = 0; i < total; ++i) parent[i] = NOT_VISITED;

    Queue waiting(64);
    parent[startIdx] = NO_PARENT;
    waiting.push(startIdx);
    while (!waiting.empty()) {
        int cur = waiting.pop();
        ++result.explored;
        if (cur == goalIdx) { result.found = true; break; }
        int row = cur / cols, col = cur % cols;
        for (int d = 0; d < 4; ++d) {
            int nr = row + DIR_R[d], nc = col + DIR_C[d];
            if (m.isWall(nr, nc)) continue;
            int next = nr * cols + nc;
            if (parent[next] != NOT_VISITED) continue;     // already reached
            parent[next] = cur;                            // mark when we add it to the queue
            waiting.push(next);
        }
    }
    if (result.found) buildPath(m, parent, result);
    delete[] parent;
}

// DFS: goes as deep as possible before trying another branch. The path it
// finds is valid but not always the shortest.
// Each stack entry is a pair of ints: the parent first, then the cell on top.
inline void dfsOnce(const Maze& m, SolveResult& result) {
    result.clear();
    Cell s = m.start(), g = m.goal();
    if (m.isWall(s.r, s.c) || m.isWall(g.r, g.c)) return;

    int cols = m.cols(), total = m.rows() * cols;
    int startIdx = s.r * cols + s.c, goalIdx = g.r * cols + g.c;
    int* parent = new int[total];
    for (int i = 0; i < total; ++i) parent[i] = NOT_VISITED;

    Stack waiting(64);
    waiting.push(NO_PARENT);
    waiting.push(startIdx);
    while (!waiting.empty()) {
        int cur = waiting.pop();
        int from = waiting.pop();
        if (parent[cur] != NOT_VISITED) continue;      // reached earlier by another route
        parent[cur] = from;
        ++result.explored;
        if (cur == goalIdx) { result.found = true; break; }
        int row = cur / cols, col = cur % cols;
        for (int d = 3; d >= 0; --d) {                 // backwards, so "up" is tried first
            int nr = row + DIR_R[d], nc = col + DIR_C[d];
            if (m.isWall(nr, nc)) continue;
            int next = nr * cols + nc;
            if (parent[next] == NOT_VISITED) {
                waiting.push(cur);
                waiting.push(next);
            }
        }
    }
    if (result.found) buildPath(m, parent, result);
    delete[] parent;
}

// Average time of one run, in microseconds.
inline double averageMicros(clock_t begin, clock_t end, int reps) {
    return (double)(end - begin) * 1000000.0 / CLOCKS_PER_SEC / reps;
}

// reps = how many times to repeat the solve so the time can be averaged.
inline void solveBFS(const Maze& m, SolveResult& result, int reps = 1) {
    if (reps < 1) reps = 1;
    clock_t begin = clock();
    for (int i = 0; i < reps; ++i) bfsOnce(m, result);
    result.micros = averageMicros(begin, clock(), reps);
}

inline void solveDFS(const Maze& m, SolveResult& result, int reps = 1) {
    if (reps < 1) reps = 1;
    clock_t begin = clock();
    for (int i = 0; i < reps; ++i) dfsOnce(m, result);
    result.micros = averageMicros(begin, clock(), reps);
}

// Fills mark[rows*cols] with 1 on every cell of the path and 0 everywhere else.
inline void markPath(const Maze& m, const SolveResult& result, char* mark) {
    for (int i = 0; i < m.rows() * m.cols(); ++i) mark[i] = 0;
    for (int i = 0; i < result.pathCells; ++i)
        mark[result.path[i].r * m.cols() + result.path[i].c] = 1;
}
