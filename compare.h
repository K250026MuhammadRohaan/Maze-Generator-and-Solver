// compare.h - Module 3: generate-and-solve demo and algorithm comparison.
#pragma once
#include <stdio.h>
#include <time.h>
#include "maze.h"
#include "solvers.h"
#include "ui.h"
#include "utils.h"

// Each test is repeated many times and averaged, because one run is too fast to time.
const int GEN_REPS = 200;
const int SOLVE_REPS = 1000;

// Counts rooms that have exactly one open neighbour (a dead end).
inline int countDeadEnds(const Maze& m) {
    int n = 0;
    for (int r = 1; r < m.rows(); r += 2) {
        for (int c = 1; c < m.cols(); c += 2) {
            if (m.isWall(r, c)) continue;
            int neighbours = 0;
            if (m.isOpen(r - 1, c)) ++neighbours;
            if (m.isOpen(r + 1, c)) ++neighbours;
            if (m.isOpen(r, c - 1)) ++neighbours;
            if (m.isOpen(r, c + 1)) ++neighbours;
            if (neighbours == 1) ++n;
        }
    }
    return n;
}

// Average time (microseconds) to generate a maze with the chosen generator.
inline double timeGeneration(Maze& m, int generator, int reps) {
    clock_t begin = clock();
    for (int i = 0; i < reps; ++i) generateMaze(m, generator);
    return averageMicros(begin, clock(), reps);
}

inline void printSolveHeader() {
    printf("  %-6s%8s%10s%10s%12s\n", "Solver", "Found", "PathLen", "Explored", "Time(us)");
}

inline void printSolveRow(const char* label, const SolveResult& r) {
    printf("  %-6s%8s%10d%10d%12.2f\n", label, r.found ? "yes" : "no",
           r.found ? r.pathLength : 0, r.explored, r.micros);
}

// Menu option: generate one maze, show it, solve it with both solvers.
inline void generateAndSolve() {
    MazeConfig cfg;
    if (!askMazeConfig(cfg, true)) return;
    Maze m(cfg.rows, cfg.cols);
    buildMaze(m, cfg);
    printf("\nGenerated maze (%dx%d, %s%s):\n\n", m.rows(), m.cols(),
           generatorName(cfg.generator), cfg.loops ? ", with loops" : "");
    m.display();

    SolveResult bfs, dfs;
    solveBFS(m, bfs, SOLVE_REPS);
    solveDFS(m, dfs, SOLVE_REPS);

    char* mark = new char[m.rows() * m.cols()];
    if (bfs.found) {
        markPath(m, bfs, mark);
        printf("\nBFS path (shortest):\n\n");
        m.display(NULL, mark);
    }
    if (dfs.found) {
        markPath(m, dfs, mark);
        printf("\nDFS path:\n\n");
        m.display(NULL, mark);
    }
    delete[] mark;

    printf("\n");
    printSolveHeader();
    printSolveRow("BFS", bfs);
    printSolveRow("DFS", dfs);
}

// Menu option: compare the two generators, then the two solvers.
inline void compareAlgorithms() {
    printf("\n=== Generation comparison (average of %d runs) ===\n", GEN_REPS);
    printf("  %-10s%14s%14s%12s%14s\n", "Size", "DFS (us)", "Kruskal (us)", "DFS dead", "Kruskal dead");
    const int sizes[5] = {11, 21, 31, 51, 101};
    for (int i = 0; i < 5; ++i) {
        int s = sizes[i];
        Maze m(s, s);
        double dfsTime = timeGeneration(m, GEN_DFS, GEN_REPS);
        int dfsDead = countDeadEnds(m);
        double kruskalTime = timeGeneration(m, GEN_KRUSKAL, GEN_REPS);
        int kruskalDead = countDeadEnds(m);
        printf("  %3dx%-6d%14.1f%14.1f%12d%14d\n", s, s, dfsTime, kruskalTime, dfsDead, kruskalDead);
    }

    int choice = readInt("\nSolver comparison size - 1) 11x11  2) 21x21  3) 31x31: ", 1, 3);
    int size = 11 + 10 * (choice - 1);
    printf("\n=== Solver comparison on %dx%d mazes (each pair solves the same maze, %d runs averaged) ===\n",
           size, size, SOLVE_REPS);
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int loops = 0; loops <= 1; ++loops) {
            MazeConfig cfg;
            cfg.rows = size;
            cfg.cols = size;
            cfg.generator = gen;
            cfg.loops = (loops == 1);
            Maze m(size, size);
            buildMaze(m, cfg);

            SolveResult bfs, dfs;
            solveBFS(m, bfs, SOLVE_REPS);
            solveDFS(m, dfs, SOLVE_REPS);
            printf("\n  Generator: %s%s\n", generatorName(gen), loops ? " + loops" : " (perfect maze)");
            printSolveHeader();
            printSolveRow("BFS", bfs);
            printSolveRow("DFS", dfs);
        }
    }
    printf("\nNote: in a perfect maze both solvers give the same path length;\n"
           "with loops BFS is never longer than DFS.\n");
}
