// Maze Generator and Solver - Data Structures project (BAI-3A)
// Build: g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o maze
#include <iostream>
#include <memory>
#include "compare.h"
#include "game.h"
#include "kruskal.h"
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"
#include "tests.h"
#include "ui.h"

static void leaderboardMenu(Leaderboard& lb) {
    for (;;) {
        bool exists = lb.load();
        std::cout << "\n=== LEADERBOARD ===\n";
        if (!exists) std::cout << "  (" << lb.path() << " not found yet - play a game to create it)\n";
        if (lb.skippedLines() > 0) std::cout << "  Note: " << lb.skippedLines() << " malformed line(s) ignored.\n";
        std::cout << "  1) Top five by score (insertion sort)\n"
                     "  2) Alphabetical (merge sort)\n"
                     "  3) Search for a player (linear + binary search)\n"
                     "  0) Back\n";
        int ch = readInt("Choose: ", 0, 3);
        if (ch == 0) return;
        std::unique_ptr<Entry[]> a = lb.copyRecords();
        int n = lb.size();
        if (ch == 1) {
            insertionSort(a.get(), n, SortKey::Score);
            Leaderboard::printTable(a.get(), n);
        } else if (ch == 2) {
            mergeSort(a.get(), n, SortKey::Name);
            Leaderboard::printTable(a.get(), n);
        } else {
            std::string name;
            readLine("Player name: ", name);
            int li = linearSearchName(a.get(), n, name);      // a is in score order
            mergeSort(a.get(), n, SortKey::Name);
            int bi = binarySearchName(a.get(), n, name);      // a is now in name order
            if (li < 0 || bi < 0) {
                std::cout << "  No player named \"" << name << "\" on the leaderboard.\n";
            } else {
                std::cout << "  Found: " << a[bi].name << " - score " << a[bi].score
                          << ", " << a[bi].moves << " moves, " << a[bi].seconds << "s (rank "
                          << (li + 1) << " by score)\n";
            }
        }
    }
}

int main() {
    Leaderboard lb("scores.txt");
    try {
        for (;;) {
            std::cout << "\n=============================\n"
                         "   MAZE GENERATOR AND SOLVER\n"
                         "=============================\n"
                         "  1) Play maze game\n"
                         "  2) Generate and solve a maze\n"
                         "  3) Compare algorithms\n"
                         "  4) Leaderboard\n"
                         "  5) Run tests\n"
                         "  0) Exit\n";
            int ch = readInt("Choose: ", 0, 5);
            try {
                switch (ch) {
                    case 0: std::cout << "Goodbye!\n"; return 0;
                    case 1: playGame(lb); break;
                    case 2: generateAndSolve(); break;
                    case 3: compareAlgorithms(); break;
                    case 4: leaderboardMenu(lb); break;
                    case 5: runAllTests(); break;
                }
            } catch (const std::invalid_argument& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
            if (ch != 4) pauseForEnter();
        }
    } catch (const std::exception& e) {
        std::cout << "\nExiting (" << e.what() << ").\n";
    }
    return 0;
}
