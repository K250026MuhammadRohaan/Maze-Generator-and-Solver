// ui.h - small console helpers + maze configuration shared by menu modules
#pragma once
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include "kruskal.h"
#include "maze.h"

constexpr int GEN_DFS = 1;
constexpr int GEN_KRUSKAL = 2;

inline void clearScreen() {
#ifdef _WIN32
    (void)!std::system("cls");
#else
    (void)!std::system("clear");
#endif
}

inline bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt << std::flush;
    return (bool)std::getline(std::cin, out);
}

// Keeps asking until the user enters an integer in [lo, hi]. Throws on EOF.
inline int readInt(const std::string& prompt, int lo, int hi) {
    for (;;) {
        std::string s;
        if (!readLine(prompt, s)) throw std::runtime_error("input closed");
        try {
            size_t used = 0;
            int v = std::stoi(s, &used);
            if (used == s.size() || s.find_first_not_of(" \t\r", used) == std::string::npos) {
                if (v >= lo && v <= hi) return v;
            }
        } catch (...) {}
        std::cout << "  Please enter a number from " << lo << " to " << hi << ".\n";
    }
}

inline bool readYesNo(const std::string& prompt) {
    for (;;) {
        std::string s;
        if (!readLine(prompt + " (y/n): ", s)) throw std::runtime_error("input closed");
        if (!s.empty() && (s[0] == 'y' || s[0] == 'Y')) return true;
        if (!s.empty() && (s[0] == 'n' || s[0] == 'N')) return false;
    }
}

inline void pauseForEnter() {
    std::string s;
    readLine("\nPress Enter to continue...", s);
}

// ----------------------------------------------------------- generation ----
struct MazeConfig {
    int rows = 11, cols = 11;
    int generator = GEN_DFS;
    bool loops = false;
    int difficulty = 1;   // 1 Easy, 2 Medium, 3 Hard, 4 Custom
};

inline void generateMaze(Maze& m, int generator) {
    if (generator == GEN_KRUSKAL) generateKruskal(m);
    else generateDFS(m);
}

inline int loopCountFor(const Maze& m) {
    int k = m.rooms() / 8;
    return k < 1 ? 1 : k;
}

inline Maze buildMaze(const MazeConfig& cfg) {
    Maze m(cfg.rows, cfg.cols);          // throws std::invalid_argument on bad size
    generateMaze(m, cfg.generator);
    if (cfg.loops) addLoops(m, loopCountFor(m));
    return m;
}

inline const char* generatorName(int g) { return g == GEN_KRUSKAL ? "Kruskal" : "DFS"; }

// Returns false if the user's custom size was invalid.
inline bool askMazeConfig(MazeConfig& cfg, bool allowCustom) {
    std::cout << "\nDifficulty:\n  1) Easy   (11 x 11)\n  2) Medium (21 x 21)\n  3) Hard   (31 x 31)\n";
    if (allowCustom) std::cout << "  4) Custom size\n";
    cfg.difficulty = readInt("Choose: ", 1, allowCustom ? 4 : 3);
    switch (cfg.difficulty) {
        case 1: cfg.rows = cfg.cols = 11; break;
        case 2: cfg.rows = cfg.cols = 21; break;
        case 3: cfg.rows = cfg.cols = 31; break;
        default:
            cfg.rows = readInt("Rows (odd, 5-101): ", -1000, 1000);
            cfg.cols = readInt("Cols (odd, 5-101): ", -1000, 1000);
            if (!Maze::validSize(cfg.rows, cfg.cols)) {
                std::cout << "  Invalid size: rows and columns must be odd numbers from 5 to 101.\n";
                return false;
            }
    }
    std::cout << "\nGeneration algorithm:\n  1) DFS with backtracking\n  2) Kruskal's (Union-Find)\n";
    cfg.generator = readInt("Choose: ", 1, 2);
    cfg.loops = readYesNo("Add loops (extra routes)?");
    return true;
}
