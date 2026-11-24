// game.h - Module 1: interactive gameplay (WASD + Enter), timing, scoring.
#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"
#include "ui.h"

// Higher is better. Base depends on difficulty; time and wasted moves cost points.
inline int calculateScore(int difficulty, int seconds, int moves, int optimalSteps) {
    int mult = difficulty >= 1 && difficulty <= 3 ? difficulty : 2;
    int base = 1000 * mult;
    int wasted = moves > optimalSteps ? moves - optimalSteps : 0;
    int score = base - seconds * 2 - wasted * 5;
    return score < 0 ? 0 : score;
}

inline void playGame(Leaderboard& lb) {
    MazeConfig cfg;
    if (!askMazeConfig(cfg, false)) return;
    Maze m = buildMaze(cfg);
    SolveResult opt = solveBFS(m);
    Cell p = m.start(), ex = m.exit();
    int moves = 0;
    std::string msg = "Find the exit (E).";
    auto t0 = std::chrono::steady_clock::now();
    bool won = false;

    while (!won) {
        int elapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::steady_clock::now() - t0).count();
        clearScreen();
        std::cout << "=== MAZE GAME (" << m.rows() << "x" << m.cols() << ", "
                  << generatorName(cfg.generator) << ") ===   Moves: " << moves
                  << "   Time: " << elapsed << "s\n\n";
        m.display(&p);
        std::cout << "\n" << msg << "\n";
        std::string line;
        if (!readLine("Move (W/A/S/D then Enter, Q to quit): ", line)) return;
        msg.clear();
        for (char ch : line) {
            char u = (char)std::toupper((unsigned char)ch);
            if (u == 'Q') { std::cout << "Game abandoned.\n"; return; }
            if (u != 'W' && u != 'A' && u != 'S' && u != 'D') continue;
            if (tryMove(m, p, u)) {
                ++moves;
                if (p.r == ex.r && p.c == ex.c) { won = true; break; }
            } else {
                msg = "You can't move through a wall!";
            }
        }
    }

    int seconds = (int)std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::steady_clock::now() - t0).count();
    int score = calculateScore(cfg.difficulty, seconds, moves, opt.pathLength);
    clearScreen();
    m.display(&p);
    std::cout << "\n*** You reached the exit! ***\n"
              << "Moves: " << moves << " (shortest possible: " << opt.pathLength << ")\n"
              << "Time:  " << seconds << "s\nScore: " << score << "\n\n";

    std::string name;
    readLine("Enter your name for the leaderboard: ", name);
    lb.load();
    lb.add(Entry{name, score, moves, seconds});
    if (!lb.save()) std::cout << "Warning: could not write " << lb.path() << "\n";
    std::cout << "\nTop scores:\n";
    lb.print();
}
