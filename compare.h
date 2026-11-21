// compare.h - Module 3: generate-and-solve demo and algorithm comparison.
#pragma once
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include "maze.h"
#include "solvers.h"
#include "ui.h"

// Open rooms with exactly one open neighbour.
inline int countDeadEnds(const Maze& m) {
    int n = 0;
    for (int r = 1; r < m.rows(); r += 2)
        for (int c = 1; c < m.cols(); c += 2) {
            if (m.isWall(r, c)) continue;
            int deg = (int)m.isOpen(r - 1, c) + (int)m.isOpen(r + 1, c) +
                      (int)m.isOpen(r, c - 1) + (int)m.isOpen(r, c + 1);
            if (deg == 1) ++n;
        }
    return n;
}

inline double timeGeneration(Maze& m, int generator, int reps) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < reps; ++i) generateMaze(m, generator);
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(t1 - t0).count() / reps;
}

inline void printSolveRow(const char* label, const SolveResult& r) {
    std::cout << "  " << std::left << std::setw(6) << label << std::right
              << std::setw(8) << (r.found ? "yes" : "no")
              << std::setw(10) << (r.found ? r.pathLength : 0)
              << std::setw(10) << r.explored
              << std::setw(12) << std::fixed << std::setprecision(2) << r.micros << "\n";
}

inline void printSolveHeader() {
    std::cout << "  " << std::left << std::setw(6) << "Solver" << std::right << std::setw(8) << "Found"
              << std::setw(10) << "PathLen" << std::setw(10) << "Explored" << std::setw(12) << "Time(us)" << "\n";
}

// Menu option: generate one maze, show it, solve with both solvers.
inline void generateAndSolve() {
    MazeConfig cfg;
    if (!askMazeConfig(cfg, true)) return;
    Maze m = buildMaze(cfg);
    std::cout << "\nGenerated maze (" << m.rows() << "x" << m.cols() << ", " << generatorName(cfg.generator)
              << (cfg.loops ? ", with loops" : "") << "):\n\n";
    m.display();

    SolveResult bfs = solveBFS(m, 100), dfs = solveDFS(m, 100);
    std::unique_ptr<char[]> mark(new char[m.rows() * m.cols()]);
    if (bfs.found) {
        markPath(m, bfs, mark.get());
        std::cout << "\nBFS path (shortest):\n\n";
        m.display(nullptr, mark.get());
    }
    if (dfs.found) {
        markPath(m, dfs, mark.get());
        std::cout << "\nDFS path:\n\n";
        m.display(nullptr, mark.get());
    }
    std::cout << "\n";
    printSolveHeader();
    printSolveRow("BFS", bfs);
    printSolveRow("DFS", dfs);
}

// Menu option: compare generators across sizes and solvers on the same mazes.
inline void compareAlgorithms() {
    std::cout << "\n=== Generation comparison (average of 20 runs) ===\n";
    std::cout << "  " << std::left << std::setw(10) << "Size" << std::right << std::setw(14) << "DFS (us)"
              << std::setw(14) << "Kruskal (us)" << std::setw(12) << "DFS dead" << std::setw(14) << "Kruskal dead" << "\n";
    const int sizes[] = {11, 21, 31, 51, 101};
    for (int s : sizes) {
        Maze m(s, s);
        double td = timeGeneration(m, GEN_DFS, 20);
        int dd = countDeadEnds(m);
        double tk = timeGeneration(m, GEN_KRUSKAL, 20);
        int kd = countDeadEnds(m);
        std::cout << "  " << std::left << std::setw(10) << (std::to_string(s) + "x" + std::to_string(s))
                  << std::right << std::fixed << std::setprecision(1) << std::setw(14) << td
                  << std::setw(14) << tk << std::setw(12) << dd << std::setw(14) << kd << "\n";
    }

    int diff = readInt("\nSolver comparison size - 1) 11x11  2) 21x21  3) 31x31: ", 1, 3);
    int size = diff == 1 ? 11 : diff == 2 ? 21 : 31;
    std::cout << "\n=== Solver comparison on " << size << "x" << size
              << " mazes (each row pair solves the same maze, 50 runs averaged) ===\n";
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int loops = 0; loops <= 1; ++loops) {
            MazeConfig cfg;
            cfg.rows = cfg.cols = size; cfg.generator = gen; cfg.loops = loops == 1;
            Maze m = buildMaze(cfg);
            std::cout << "\n  Generator: " << generatorName(gen)
                      << (loops ? " + loops" : " (perfect maze)") << "\n";
            printSolveHeader();
            printSolveRow("BFS", solveBFS(m, 50));
            printSolveRow("DFS", solveDFS(m, 50));
        }
    }
    std::cout << "\nNote: in a perfect maze both solvers give the same path length;\n"
                 "with loops BFS is never longer than DFS.\n";
}
