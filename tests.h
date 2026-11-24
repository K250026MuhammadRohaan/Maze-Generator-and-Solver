// tests.h - Module 3: normal and edge-case tests, run from the menu (option 5).
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "compare.h"
#include "ds.h"
#include "game.h"
#include "kruskal.h"
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"

int g_testPass = 0;
int g_testFail = 0;

// CHECK(condition) counts a pass, or prints the failed condition and its line.
#define CHECK(cond)                                                          \
    do {                                                                     \
        if (cond) ++g_testPass;                                              \
        else { ++g_testFail; printf("    FAIL: %s (line %d)\n", #cond, __LINE__); } \
    } while (0)

// Can every open cell be reached from the start? (flood fill with our Stack)
inline bool fullyConnected(const Maze& m) {
    int cols = m.cols();
    char* seen = new char[m.rows() * cols];
    for (int i = 0; i < m.rows() * cols; ++i) seen[i] = 0;

    Stack todo;
    Cell s = m.start();
    todo.push(s.r * cols + s.c);
    seen[s.r * cols + s.c] = 1;
    int count = 0;
    while (!todo.empty()) {
        int cur = todo.pop();
        ++count;
        for (int d = 0; d < 4; ++d) {
            int nr = cur / cols + DIR_R[d], nc = cur % cols + DIR_C[d];
            if (m.isWall(nr, nc) || seen[nr * cols + nc]) continue;
            seen[nr * cols + nc] = 1;
            todo.push(nr * cols + nc);
        }
    }
    delete[] seen;
    return count == m.openCount();
}

// Is the path a real route: starts at S, ends at E, no walls, one step at a time?
inline bool validPath(const Maze& m, const SolveResult& result) {
    if (!result.found || result.pathCells < 2) return false;
    Cell s = m.start(), g = m.goal();
    Cell last = result.path[result.pathCells - 1];
    if (result.path[0].r != s.r || result.path[0].c != s.c) return false;
    if (last.r != g.r || last.c != g.c) return false;
    for (int i = 0; i < result.pathCells; ++i) {
        if (m.isWall(result.path[i].r, result.path[i].c)) return false;
        if (i > 0) {
            int step = abs(result.path[i].r - result.path[i - 1].r) +
                       abs(result.path[i].c - result.path[i - 1].c);
            if (step != 1) return false;
        }
    }
    return result.pathLength == result.pathCells - 1;
}

inline void testDataStructures() {
    printf("  [Data structures]\n");
    Stack st(2);
    for (int i = 0; i < 100; ++i) st.push(i);        // forces the array to grow
    CHECK(st.size() == 100);
    bool lifo = true;
    for (int i = 99; i >= 0; --i)
        if (st.pop() != i) lifo = false;
    CHECK(lifo);
    CHECK(st.empty());
    CHECK(st.pop() == -1);                           // empty stack gives -1

    Queue q(2);
    for (int i = 0; i < 5; ++i) q.push(i);
    q.pop();
    q.pop();                                         // front moves, so the buffer wraps around
    for (int i = 5; i < 60; ++i) q.push(i);          // and grows
    bool fifo = true;
    for (int i = 2; i < 60; ++i)
        if (q.pop() != i) fifo = false;
    CHECK(fifo);
    CHECK(q.empty());
    CHECK(q.pop() == -1);

    UnionFind uf(10);
    CHECK(uf.sets() == 10);
    CHECK(uf.unite(1, 2));
    CHECK(uf.unite(2, 3));
    CHECK(!uf.unite(1, 3));                          // already in the same group
    CHECK(uf.connected(1, 3));
    CHECK(!uf.connected(1, 4));
    CHECK(uf.sets() == 8);
}

inline void testMazeSizes() {
    printf("  [Maze size validation]\n");
    CHECK(!Maze::validSize(10, 11));
    CHECK(!Maze::validSize(11, 10));
    CHECK(!Maze::validSize(3, 3));
    CHECK(!Maze::validSize(0, 0));
    CHECK(!Maze::validSize(-5, 11));
    CHECK(!Maze::validSize(103, 11));
    CHECK(Maze::validSize(5, 5));
    CHECK(Maze::validSize(11, 21));

    Maze bad(10, 11);                                // bad size -> safe 5x5 maze
    CHECK(bad.rows() == 5 && bad.cols() == 5);
    Maze good(11, 21);
    CHECK(good.rows() == 11 && good.cols() == 21);
}

inline void testGenerators() {
    printf("  [Generators and solvers]\n");
    const int sizes[4] = {5, 11, 21, 31};
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int i = 0; i < 4; ++i) {
            for (int trial = 0; trial < 5; ++trial) {
                Maze m(sizes[i], sizes[i]);
                generateMaze(m, gen);
                CHECK(m.openConnectors() == m.rooms() - 1);      // spanning tree: no loops
                CHECK(fullyConnected(m));                        // every room reachable
                CHECK(m.isOpen(1, 1) && m.isOpen(m.goal().r, m.goal().c));
                SolveResult b, d;
                solveBFS(m, b);
                solveDFS(m, d);
                CHECK(validPath(m, b));
                CHECK(validPath(m, d));
                CHECK(b.pathLength == d.pathLength);             // only one route exists
            }
        }
    }
}

inline void testLoops() {
    printf("  [Loops]\n");
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int trial = 0; trial < 5; ++trial) {
            Maze m(21, 21);
            generateMaze(m, gen);
            int before = m.openConnectors();
            int opened = addLoops(m, loopCountFor(m));
            CHECK(opened > 0);
            CHECK(m.openConnectors() == before + opened);
            CHECK(fullyConnected(m));
            SolveResult b, d;
            solveBFS(m, b);
            solveDFS(m, d);
            CHECK(validPath(m, b));
            CHECK(validPath(m, d));
            CHECK(b.pathLength <= d.pathLength);                 // BFS is shortest
        }
    }
    Maze tiny(5, 5);
    generateDFS(tiny);
    CHECK(addLoops(tiny, 0) == 0);
    CHECK(addLoops(tiny, -3) == 0);
}

inline void testUnreachable() {
    printf("  [Unreachable exit]\n");
    Maze m(11, 11);                       // all walls
    SolveResult b, d;
    solveBFS(m, b);
    solveDFS(m, d);
    CHECK(!b.found);                      // the start itself is a wall
    CHECK(!d.found);

    m.setOpen(1, 1);
    m.setOpen(m.goal().r, m.goal().c);    // both ends open, but no route between them
    solveBFS(m, b);
    solveDFS(m, d);
    CHECK(!b.found && b.pathCells == 0 && b.explored == 1);
    CHECK(!d.found && d.pathCells == 0 && d.explored == 1);
}

inline void testMovement() {
    printf("  [Movement / collisions]\n");
    Maze m(11, 11);
    generateDFS(m);
    Cell p = m.start();
    CHECK(!tryMove(m, p, 'W') && p.r == 1 && p.c == 1);    // border wall above
    CHECK(!tryMove(m, p, 'A') && p.r == 1 && p.c == 1);    // border wall on the left
    CHECK(!tryMove(m, p, 'X'));                            // not a movement key
    CHECK(!tryMove(m, p, ' '));
    bool moved = tryMove(m, p, 'd') || tryMove(m, p, 'S'); // one of them is open at the start
    CHECK(moved);
    CHECK(tryMove(m, p, p.c > 1 ? 'a' : 'w'));             // step back
    CHECK(p.r == 1 && p.c == 1);
    Cell q = m.goal();
    CHECK(!tryMove(m, q, 'D') && !tryMove(m, q, 'S'));     // cannot leave through the border
}

inline void testScoring() {
    printf("  [Scoring]\n");
    CHECK(calculateScore(1, 0, 20, 20) == 1000);
    CHECK(calculateScore(2, 10, 30, 20) == 2000 - 20 - 50);
    CHECK(calculateScore(1, 100000, 500, 10) == 0);        // never below 0
    CHECK(calculateScore(3, 0, 5, 20) == 3000);            // fewer moves than shortest: no penalty
}

inline void testText() {
    printf("  [Text helpers]\n");
    int v = 0;
    CHECK(parseInt("42", v) && v == 42);
    CHECK(parseInt("  -7  ", v) && v == -7);
    CHECK(!parseInt("", v));
    CHECK(!parseInt("abc", v));
    CHECK(!parseInt("12x", v));
    CHECK(!parseInt("99999999999", v));
    CHECK(compareNoCase("Bob", "bOB") == 0);
    CHECK(compareNoCase("abc", "abd") < 0);
    CHECK(compareNoCase("b", "abc") > 0);
    CHECK(compareNoCase("", "a") < 0);
}

inline void testLeaderboard() {
    printf("  [Leaderboard]\n");
    const char* file = "test_scores.txt";
    remove(file);
    Leaderboard lb(file);
    CHECK(!lb.load());                    // missing file: empty board, no crash
    CHECK(lb.size() == 0);

    FILE* out = fopen(file, "w");
    fputs("Ann 500 30 40\nthis is garbage\nBob 900 25 20\nCy -5 1 1\n\n"
          "Dee 700 10 10\r\nEli 100 5 5\nFay 800 12 12\nGus 600 9 9\nHal 50 abc 1\n", out);
    fclose(out);
    CHECK(lb.load());
    CHECK(lb.size() == 5);                // six good lines, only the top five kept
    CHECK(lb.skippedLines() == 3);        // garbage, negative score, non-number
    CHECK(lb.at(0).score == 900 && compareNoCase(lb.at(0).name, "Bob") == 0);
    CHECK(compareNoCase(lb.at(1).name, "Fay") == 0 && compareNoCase(lb.at(2).name, "Dee") == 0);
    CHECK(compareNoCase(lb.at(4).name, "Ann") == 0);     // Eli (100) was dropped

    lb.add("Zed Zed", 1000, 5, 5);
    CHECK(lb.size() == 5);
    CHECK(compareNoCase(lb.at(0).name, "Zed_Zed") == 0); // space became '_'
    CHECK(compareNoCase(lb.at(4).name, "Gus") == 0);
    lb.add("Low", 1, 1, 1);               // too low to get on the board
    CHECK(lb.size() == 5 && compareNoCase(lb.at(4).name, "Gus") == 0);
    lb.add("", 650, 1, 1);                // empty name gets a default
    CHECK(compareNoCase(lb.at(4).name, "Player") == 0 || compareNoCase(lb.at(3).name, "Player") == 0);
    lb.add("ABCDEFGHIJKLMNOPQRST", 5000, 1, 1);   // long names are cut to 15 letters
    CHECK(textLength(lb.at(0).name) == 15);

    CHECK(lb.save());
    Leaderboard lb2(file);
    CHECK(lb2.load() && lb2.size() == 5 && lb2.skippedLines() == 0);
    bool same = true;
    for (int i = 0; i < 5; ++i)
        if (compareNoCase(lb.at(i).name, lb2.at(i).name) != 0 || lb.at(i).score != lb2.at(i).score) same = false;
    CHECK(same);

    // Insertion sort and merge sort must give the same result.
    const int N = 40;
    Entry a[N], b[N];
    for (int i = 0; i < N; ++i) {
        char name[NAME_SIZE];
        sprintf(name, "p%d", (i * 17) % N);
        copyText(a[i].name, name, NAME_SIZE);
        a[i].score = (i * 37) % 23;
        a[i].seconds = (i * 11) % 7;
        b[i] = a[i];
    }
    insertionSort(a, N, BY_SCORE);
    mergeSort(b, N, BY_SCORE);
    bool ok = true, sorted = true;
    for (int i = 0; i < N; ++i)
        if (compareNoCase(a[i].name, b[i].name) != 0 || a[i].score != b[i].score) ok = false;
    for (int i = 1; i < N; ++i)
        if (a[i - 1].score < a[i].score) sorted = false;
    CHECK(ok && sorted);

    mergeSort(a, N, BY_NAME);
    int found = binarySearchName(a, N, "P7");              // search ignores upper/lower case
    CHECK(found >= 0 && compareNoCase(a[found].name, "p7") == 0);
    CHECK(found == linearSearchName(a, N, "p7"));
    CHECK(binarySearchName(a, N, "nobody") == -1);
    CHECK(linearSearchName(a, N, "nobody") == -1);
    CHECK(binarySearchName(a, 0, "x") == -1);              // empty array

    remove(file);
}

inline void runAllTests() {
    g_testPass = 0;
    g_testFail = 0;
    printf("\n=== Running tests ===\n");
    testDataStructures();
    testText();
    testMazeSizes();
    testGenerators();
    testLoops();
    testUnreachable();
    testMovement();
    testScoring();
    testLeaderboard();
    printf("\nResult: %d passed, %d failed.\n", g_testPass, g_testFail);
}
