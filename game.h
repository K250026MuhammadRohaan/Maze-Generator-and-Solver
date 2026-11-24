// game.h - Module 1: the playable game (WASD + Enter), timer and score.
#pragma once
#include <stdio.h>
#include <time.h>
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"
#include "ui.h"
#include "utils.h"

// Higher is better. The difficulty sets the starting points; every second
// costs 2 points and every move beyond the shortest path costs 5 points.
inline int calculateScore(int difficulty, int seconds, int moves, int shortestSteps) {
    int level = (difficulty >= 1 && difficulty <= 3) ? difficulty : 2;   // custom size counts as medium
    int wasted = moves > shortestSteps ? moves - shortestSteps : 0;
    int score = 1000 * level - seconds * 2 - wasted * 5;
    return score < 0 ? 0 : score;
}

inline void playGame(Leaderboard& lb) {
    MazeConfig cfg;
    if (!askMazeConfig(cfg, false)) return;
    Maze m(cfg.rows, cfg.cols);
    buildMaze(m, cfg);

    SolveResult shortest;
    solveBFS(m, shortest);                 // the best possible number of moves
    Cell player = m.start(), goal = m.goal();
    int moves = 0;
    bool won = false;
    const char* message = "Find the exit (E).";
    time_t startTime = time(NULL);

    while (!won) {
        int elapsed = (int)difftime(time(NULL), startTime);
        clearScreen();
        printf("=== MAZE GAME (%dx%d, %s) ===   Moves: %d   Time: %ds\n\n",
               m.rows(), m.cols(), generatorName(cfg.generator), moves, elapsed);
        m.display(&player);
        printf("\n%s\n", message);

        char line[LINE_SIZE];
        if (!readLine("Move (W/A/S/D then Enter, Q to quit): ", line, LINE_SIZE)) return;
        message = "";
        for (int i = 0; line[i] != '\0'; ++i) {          // several keys may be typed at once
            char key = toUpperChar(line[i]);
            if (key == 'Q') {
                printf("Game abandoned.\n");
                return;
            }
            if (key != 'W' && key != 'A' && key != 'S' && key != 'D') continue;
            if (tryMove(m, player, key)) {
                ++moves;
                if (player.r == goal.r && player.c == goal.c) {
                    won = true;
                    break;
                }
            } else {
                message = "You can't move through a wall!";
            }
        }
    }

    int seconds = (int)difftime(time(NULL), startTime);
    int score = calculateScore(cfg.difficulty, seconds, moves, shortest.pathLength);
    clearScreen();
    m.display(&player);
    printf("\n*** You reached the exit! ***\n");
    printf("Moves: %d (shortest possible: %d)\n", moves, shortest.pathLength);
    printf("Time:  %ds\nScore: %d\n\n", seconds, score);

    char name[LINE_SIZE];
    readLine("Enter your name for the leaderboard: ", name, LINE_SIZE);
    lb.load();
    lb.add(name, score, moves, seconds);
    if (!lb.save()) printf("Warning: could not write %s\n", lb.path());
    printf("\nTop scores:\n");
    lb.print();
}
