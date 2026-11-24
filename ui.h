// ui.h - maze settings (difficulty, generator, loops) shared by the menus.
#pragma once
#include <stdio.h>
#include "kruskal.h"
#include "maze.h"
#include "utils.h"

const int GEN_DFS = 1;
const int GEN_KRUSKAL = 2;

struct MazeConfig {
    int rows, cols;
    int generator;      // GEN_DFS or GEN_KRUSKAL
    bool loops;
    int difficulty;     // 1 Easy, 2 Medium, 3 Hard, 4 Custom
    MazeConfig() : rows(11), cols(11), generator(GEN_DFS), loops(false), difficulty(1) {}
};

inline void generateMaze(Maze& m, int generator) {
    if (generator == GEN_KRUSKAL) generateKruskal(m);
    else generateDFS(m);
}

// How many extra walls to open: one per 8 rooms, at least one.
inline int loopCountFor(const Maze& m) {
    int k = m.rooms() / 8;
    return k < 1 ? 1 : k;
}

// Fills m (already the right size) following the settings in cfg.
inline void buildMaze(Maze& m, const MazeConfig& cfg) {
    generateMaze(m, cfg.generator);
    if (cfg.loops) addLoops(m, loopCountFor(m));
}

inline const char* generatorName(int g) { return g == GEN_KRUSKAL ? "Kruskal" : "DFS"; }

// Asks the user for the settings. Returns false if the custom size was invalid.
inline bool askMazeConfig(MazeConfig& cfg, bool allowCustom) {
    printf("\nDifficulty:\n  1) Easy   (11 x 11)\n  2) Medium (21 x 21)\n  3) Hard   (31 x 31)\n");
    if (allowCustom) printf("  4) Custom size\n");
    cfg.difficulty = readInt("Choose: ", 1, allowCustom ? 4 : 3);

    if (cfg.difficulty == 4) {
        cfg.rows = readInt("Rows (odd, 5-101): ", -1000, 1000);
        cfg.cols = readInt("Cols (odd, 5-101): ", -1000, 1000);
        if (!Maze::validSize(cfg.rows, cfg.cols)) {
            printf("  Invalid size: rows and columns must be odd numbers from 5 to 101.\n");
            return false;
        }
    } else {
        cfg.rows = cfg.cols = 11 + 10 * (cfg.difficulty - 1);     // 11, 21 or 31
    }

    printf("\nGeneration algorithm:\n  1) DFS with backtracking\n  2) Kruskal's (Union-Find)\n");
    cfg.generator = readInt("Choose: ", 1, 2);
    cfg.loops = readYesNo("Add loops (extra routes)?");
    return true;
}
