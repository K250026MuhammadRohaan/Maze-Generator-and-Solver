// Maze Generator and Solver - Data Structures project (BAI-3A)
// Build: g++ -O2 -Wall -Wextra main.cpp -o maze
#include <stdio.h>
#include "compare.h"
#include "game.h"
#include "kruskal.h"
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"
#include "tests.h"
#include "ui.h"
#include "utils.h"

static void leaderboardMenu(Leaderboard& lb) {
    for (;;) {
        bool exists = lb.load();
        printf("\n=== LEADERBOARD ===\n");
        if (!exists) printf("  (%s not found yet - play a game to create it)\n", lb.path());
        if (lb.skippedLines() > 0) printf("  Note: %d malformed line(s) ignored.\n", lb.skippedLines());
        printf("  1) Top five by score (insertion sort)\n"
               "  2) Alphabetical (merge sort)\n"
               "  3) Search for a player (linear + binary search)\n"
               "  0) Back\n");
        int choice = readInt("Choose: ", 0, 3);
        if (choice == 0) return;

        Entry list[MAX_RECORDS];             // a copy, so sorting does not change the board
        int n = lb.copyTo(list);
        if (choice == 1) {
            insertionSort(list, n, BY_SCORE);
            Leaderboard::printTable(list, n);
        } else if (choice == 2) {
            mergeSort(list, n, BY_NAME);
            Leaderboard::printTable(list, n);
        } else {
            char name[LINE_SIZE];
            readLine("Player name: ", name, LINE_SIZE);
            int byScore = linearSearchName(list, n, name);    // list is in score order
            mergeSort(list, n, BY_NAME);
            int byName = binarySearchName(list, n, name);     // list is now in name order
            if (byScore < 0 || byName < 0) {
                printf("  No player named \"%s\" on the leaderboard.\n", name);
            } else {
                printf("  Found: %s - score %d, %d moves, %ds (rank %d by score)\n",
                       list[byName].name, list[byName].score, list[byName].moves,
                       list[byName].seconds, byScore + 1);
            }
        }
    }
}

int main() {
    seedRandom();
    Leaderboard lb("scores.txt");
    for (;;) {
        printf("\n=============================\n"
               "   MAZE GENERATOR AND SOLVER\n"
               "=============================\n"
               "  1) Play maze game\n"
               "  2) Generate and solve a maze\n"
               "  3) Compare algorithms\n"
               "  4) Leaderboard\n"
               "  5) Run tests\n"
               "  0) Exit\n");
        int choice = readInt("Choose: ", 0, 5);
        switch (choice) {
            case 0: printf("Goodbye!\n"); return 0;
            case 1: playGame(lb); break;
            case 2: generateAndSolve(); break;
            case 3: compareAlgorithms(); break;
            case 4: leaderboardMenu(lb); break;
            case 5: runAllTests(); break;
        }
        if (choice != 4) pauseForEnter();
    }
}
