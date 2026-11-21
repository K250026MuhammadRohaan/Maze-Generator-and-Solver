// solvers.h - Module 3: BFS (custom Queue) and iterative DFS (custom Stack)
//             solvers with visited/parent tracking and path reconstruction.
#pragma once
#include <chrono>
#include <memory>
#include "ds.h"
#include "maze.h"

struct SolveResult {
    bool found = false;
    int pathLength = 0;                 // steps from start to exit
    int pathCells = 0;                  // cells on the path (pathLength + 1)
    int explored = 0;                   // cells taken off the queue/stack and processed
    double micros = 0;                  // average run time in microseconds
    std::unique_ptr<Cell[]> path;       // start ... exit
};

namespace solver_detail {
inline const int DR[4] = {-1, 0, 1, 0};
inline const int DC[4] = {0, 1, 0, -1};

inline void buildPath(const Maze& m, const int* parent, SolveResult& r) {
    int cols = m.cols();
    Cell ex = m.exit();
    int exitIdx = ex.r * cols + ex.c;
    int len = 0;
    for (int cur = exitIdx; cur != -1; cur = parent[cur]) ++len;
    r.path.reset(new Cell[len]);
    r.pathCells = len;
    r.pathLength = len - 1;
    int i = len - 1;
    for (int cur = exitIdx; cur != -1; cur = parent[cur]) r.path[i--] = Cell{cur / cols, cur % cols};
}

inline void reset(SolveResult& r) {
    r.found = false; r.pathLength = 0; r.pathCells = 0; r.explored = 0; r.path.reset();
}

inline void bfsOnce(const Maze& m, SolveResult& r) {
    reset(r);
    Cell s = m.start(), ex = m.exit();
    if (m.isWall(s.r, s.c) || m.isWall(ex.r, ex.c)) return;
    int n = m.rows() * m.cols(), cols = m.cols();
    std::unique_ptr<char[]> visited(new char[n]());
    std::unique_ptr<int[]> parent(new int[n]);
    for (int i = 0; i < n; ++i) parent[i] = -2;
    int startIdx = s.r * cols + s.c, exitIdx = ex.r * cols + ex.c;

    Queue<int> q(64);
    visited[startIdx] = 1; parent[startIdx] = -1;
    q.push(startIdx);
    while (!q.empty()) {
        int cur = q.pop();
        ++r.explored;
        if (cur == exitIdx) { r.found = true; break; }
        int cr = cur / cols, cc = cur % cols;
        for (int d = 0; d < 4; ++d) {
            int nr = cr + DR[d], nc = cc + DC[d];
            if (m.isWall(nr, nc)) continue;
            int ni = nr * cols + nc;
            if (visited[ni]) continue;
            visited[ni] = 1; parent[ni] = cur;
            q.push(ni);
        }
    }
    if (r.found) buildPath(m, parent.get(), r);
}

struct Frame { int idx; int parent; };

inline void dfsOnce(const Maze& m, SolveResult& r) {
    reset(r);
    Cell s = m.start(), ex = m.exit();
    if (m.isWall(s.r, s.c) || m.isWall(ex.r, ex.c)) return;
    int n = m.rows() * m.cols(), cols = m.cols();
    std::unique_ptr<char[]> visited(new char[n]());
    std::unique_ptr<int[]> parent(new int[n]);
    for (int i = 0; i < n; ++i) parent[i] = -2;
    int startIdx = s.r * cols + s.c, exitIdx = ex.r * cols + ex.c;

    Stack<Frame> st(64);
    st.push(Frame{startIdx, -1});
    while (!st.empty()) {
        Frame f = st.pop();
        if (visited[f.idx]) continue;
        visited[f.idx] = 1; parent[f.idx] = f.parent;
        ++r.explored;
        if (f.idx == exitIdx) { r.found = true; break; }
        int cr = f.idx / cols, cc = f.idx % cols;
        for (int d = 3; d >= 0; --d) {
            int nr = cr + DR[d], nc = cc + DC[d];
            if (m.isWall(nr, nc)) continue;
            int ni = nr * cols + nc;
            if (!visited[ni]) st.push(Frame{ni, f.idx});
        }
    }
    if (r.found) buildPath(m, parent.get(), r);
}

template <typename F>
SolveResult timedRun(const Maze& m, int reps, F once) {
    SolveResult r;
    if (reps < 1) reps = 1;
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < reps; ++i) once(m, r);
    auto t1 = std::chrono::steady_clock::now();
    r.micros = std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
    return r;
}
}  // namespace solver_detail

// BFS: finds a shortest path (fewest steps). O(n) time, O(n) space.
inline SolveResult solveBFS(const Maze& m, int reps = 1) {
    return solver_detail::timedRun(m, reps, solver_detail::bfsOnce);
}
// DFS: finds a valid path, not necessarily the shortest. O(n) time, O(n) space.
inline SolveResult solveDFS(const Maze& m, int reps = 1) {
    return solver_detail::timedRun(m, reps, solver_detail::dfsOnce);
}

// Fills mark[rows*cols] with 1 on every cell of the result's path.
inline void markPath(const Maze& m, const SolveResult& r, char* mark) {
    for (int i = 0; i < m.rows() * m.cols(); ++i) mark[i] = 0;
    for (int i = 0; i < r.pathCells; ++i) mark[r.path[i].r * m.cols() + r.path[i].c] = 1;
}
