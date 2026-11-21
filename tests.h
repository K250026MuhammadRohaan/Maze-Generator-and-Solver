// tests.h - Module 3: normal and edge-case tests, run from the menu.
#pragma once
#include <cstdio>
#include <fstream>
#include <iostream>
#include "compare.h"
#include "game.h"
#include "kruskal.h"
#include "leaderboard.h"
#include "maze.h"
#include "solvers.h"

inline int g_testPass = 0;
inline int g_testFail = 0;
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (cond) ++g_testPass;                                                  \
        else { ++g_testFail; std::cout << "    FAIL: " #cond " (line " << __LINE__ << ")\n"; } \
    } while (0)

namespace tests_detail {

template <typename F>
bool throwsInvalid(F f) {
    try { f(); } catch (const std::invalid_argument&) { return true; } catch (...) { return false; }
    return false;
}
template <typename F>
bool throwsUnderflow(F f) {
    try { f(); } catch (const std::underflow_error&) { return true; } catch (...) { return false; }
    return false;
}

// Every open cell reachable from the start? (flood fill with the custom Stack)
inline bool fullyConnected(const Maze& m) {
    int cols = m.cols(), n = m.rows() * cols;
    std::unique_ptr<char[]> seen(new char[n]());
    Stack<int> st;
    Cell s = m.start();
    st.push(s.r * cols + s.c);
    seen[s.r * cols + s.c] = 1;
    int count = 0;
    while (!st.empty()) {
        int cur = st.pop();
        ++count;
        int r = cur / cols, c = cur % cols;
        const int dr[4] = {-1, 0, 1, 0}, dc[4] = {0, 1, 0, -1};
        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d], nc = c + dc[d];
            if (m.isWall(nr, nc) || seen[nr * cols + nc]) continue;
            seen[nr * cols + nc] = 1;
            st.push(nr * cols + nc);
        }
    }
    return count == m.openCount();
}

inline bool validPath(const Maze& m, const SolveResult& r) {
    if (!r.found || r.pathCells < 2) return false;
    Cell s = m.start(), e = m.exit();
    if (r.path[0].r != s.r || r.path[0].c != s.c) return false;
    if (r.path[r.pathCells - 1].r != e.r || r.path[r.pathCells - 1].c != e.c) return false;
    for (int i = 0; i < r.pathCells; ++i) {
        if (m.isWall(r.path[i].r, r.path[i].c)) return false;
        if (i > 0) {
            int d = std::abs(r.path[i].r - r.path[i - 1].r) + std::abs(r.path[i].c - r.path[i - 1].c);
            if (d != 1) return false;
        }
    }
    return r.pathLength == r.pathCells - 1;
}

inline void testDataStructures() {
    std::cout << "  [Data structures]\n";
    Stack<int> st(2);
    for (int i = 0; i < 100; ++i) st.push(i);
    CHECK(st.size() == 100);
    bool lifo = true;
    for (int i = 99; i >= 0; --i) if (st.pop() != i) lifo = false;
    CHECK(lifo);
    CHECK(st.empty());
    CHECK(throwsUnderflow([&] { st.pop(); }));

    Queue<int> q(2);
    for (int i = 0; i < 5; ++i) q.push(i);
    q.pop(); q.pop();                          // force wrap-around, then growth
    for (int i = 5; i < 60; ++i) q.push(i);
    bool fifo = true;
    for (int i = 2; i < 60; ++i) if (q.pop() != i) fifo = false;
    CHECK(fifo);
    CHECK(q.empty());
    CHECK(throwsUnderflow([&] { q.pop(); }));

    UnionFind uf(10);
    CHECK(uf.sets() == 10);
    CHECK(uf.unite(1, 2));
    CHECK(uf.unite(2, 3));
    CHECK(!uf.unite(1, 3));                    // already connected
    CHECK(uf.connected(1, 3));
    CHECK(!uf.connected(1, 4));
    CHECK(uf.sets() == 8);
}

inline void testMazeSizes() {
    std::cout << "  [Maze size validation]\n";
    CHECK(throwsInvalid([] { Maze m(10, 11); }));
    CHECK(throwsInvalid([] { Maze m(11, 10); }));
    CHECK(throwsInvalid([] { Maze m(3, 3); }));
    CHECK(throwsInvalid([] { Maze m(0, 0); }));
    CHECK(throwsInvalid([] { Maze m(-5, 11); }));
    CHECK(throwsInvalid([] { Maze m(103, 11); }));
    CHECK(!throwsInvalid([] { Maze m(5, 5); }));
    CHECK(!throwsInvalid([] { Maze m(11, 21); }));
}

inline void testGenerators() {
    std::cout << "  [Generators and solvers]\n";
    const int sizes[] = {5, 11, 21, 31};
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int s : sizes) {
            for (int trial = 0; trial < 5; ++trial) {
                Maze m(s, s);
                generateMaze(m, gen);
                CHECK(m.openConnectors() == m.rooms() - 1);     // spanning tree: no cycles
                CHECK(fullyConnected(m));                       // all rooms connected
                CHECK(m.isOpen(1, 1) && m.isOpen(m.exit().r, m.exit().c));
                SolveResult b = solveBFS(m), d = solveDFS(m);
                CHECK(validPath(m, b));
                CHECK(validPath(m, d));
                CHECK(b.pathLength == d.pathLength);            // one route only
            }
        }
    }
}

inline void testLoops() {
    std::cout << "  [Loops]\n";
    for (int gen = GEN_DFS; gen <= GEN_KRUSKAL; ++gen) {
        for (int trial = 0; trial < 5; ++trial) {
            Maze m(21, 21);
            generateMaze(m, gen);
            int before = m.openConnectors();
            int opened = addLoops(m, loopCountFor(m));
            CHECK(opened > 0);
            CHECK(m.openConnectors() == before + opened);
            CHECK(fullyConnected(m));
            SolveResult b = solveBFS(m), d = solveDFS(m);
            CHECK(validPath(m, b));
            CHECK(validPath(m, d));
            CHECK(b.pathLength <= d.pathLength);                // BFS is shortest
        }
    }
    Maze tiny(5, 5);
    generateDFS(tiny);
    CHECK(addLoops(tiny, 0) == 0);
    CHECK(addLoops(tiny, -3) == 0);
}

inline void testUnreachable() {
    std::cout << "  [Unreachable exit]\n";
    Maze m(11, 11);                      // all walls
    CHECK(!solveBFS(m).found);           // start itself is a wall
    CHECK(!solveDFS(m).found);
    m.setOpen(1, 1);
    Cell e = m.exit();
    m.setOpen(e.r, e.c);                 // both ends open but no route
    SolveResult b = solveBFS(m), d = solveDFS(m);
    CHECK(!b.found && b.pathCells == 0 && b.explored == 1);
    CHECK(!d.found && d.pathCells == 0 && d.explored == 1);
}

inline void testMovement() {
    std::cout << "  [Movement / collisions]\n";
    Maze m(11, 11);
    generateDFS(m);
    Cell p = m.start();
    CHECK(!tryMove(m, p, 'W') && p.r == 1 && p.c == 1);   // border wall
    CHECK(!tryMove(m, p, 'A') && p.r == 1 && p.c == 1);   // outside grid side
    CHECK(!tryMove(m, p, 'X'));                           // invalid key
    CHECK(!tryMove(m, p, ' '));
    bool moved = tryMove(m, p, 'd') || tryMove(m, p, 'S');   // one of them is open at the start
    CHECK(moved);
    CHECK(tryMove(m, p, p.c > 1 ? 'a' : 'w'));            // step back
    CHECK(p.r == 1 && p.c == 1);
    Cell e = m.exit();
    Cell q{e.r, e.c};
    CHECK(!tryMove(m, q, 'D') && !tryMove(m, q, 'S'));    // cannot leave via the border
}

inline void testScoring() {
    std::cout << "  [Scoring]\n";
    CHECK(calculateScore(1, 0, 20, 20) == 1000);
    CHECK(calculateScore(2, 10, 30, 20) == 2000 - 20 - 50);
    CHECK(calculateScore(1, 100000, 500, 10) == 0);       // never negative
    CHECK(calculateScore(3, 0, 5, 20) == 3000);           // fewer moves than optimal: no penalty
}

inline void testLeaderboard() {
    std::cout << "  [Leaderboard]\n";
    const char* file = "test_scores.txt";
    std::remove(file);
    Leaderboard lb(file);
    CHECK(!lb.load());                    // missing file -> empty board, no crash
    CHECK(lb.size() == 0);

    {
        std::ofstream out(file);
        out << "Ann 500 30 40\nthis is garbage\nBob 900 25 20\nCy -5 1 1\n\n"
               "Dee 700 10 10\r\nEli 100 5 5\nFay 800 12 12\nGus 600 9 9\nHal 50 abc 1\n";
    }
    CHECK(lb.load());
    CHECK(lb.size() == 5);                // six valid lines, top five kept
    CHECK(lb.skippedLines() == 3);        // garbage, negative score, non-numeric
    CHECK(lb.at(0).name == "Bob" && lb.at(0).score == 900);
    CHECK(lb.at(1).name == "Fay" && lb.at(2).name == "Dee");
    CHECK(lb.at(4).name == "Ann");        // Eli (100) was dropped

    lb.add(Entry{"Zed Zed", 1000, 5, 5});
    CHECK(lb.size() == 5);
    CHECK(lb.at(0).name == "Zed_Zed");    // spaces sanitised
    CHECK(lb.at(4).name == "Gus");
    lb.add(Entry{"Low", 1, 1, 1});        // too low to enter
    CHECK(lb.size() == 5 && lb.at(4).name == "Gus");
    lb.add(Entry{"", 650, 1, 1});         // empty name gets a default
    CHECK(lb.at(4).name == "Player" || lb.at(3).name == "Player");

    CHECK(lb.save());
    Leaderboard lb2(file);
    CHECK(lb2.load() && lb2.size() == 5 && lb2.skippedLines() == 0);
    bool same = true;
    for (int i = 0; i < 5; ++i)
        if (lb.at(i).name != lb2.at(i).name || lb.at(i).score != lb2.at(i).score) same = false;
    CHECK(same);

    // sorting: insertion vs merge on the same shuffled data
    const int N = 40;
    std::unique_ptr<Entry[]> a(new Entry[N]), b(new Entry[N]);
    for (int i = 0; i < N; ++i) {
        a[i].name = "p" + std::to_string((i * 17) % N);
        a[i].score = (i * 37) % 23;
        a[i].seconds = (i * 11) % 7;
        b[i] = a[i];
    }
    insertionSort(a.get(), N, SortKey::Score);
    mergeSort(b.get(), N, SortKey::Score);
    bool ok = true, sorted = true;
    for (int i = 0; i < N; ++i) if (a[i].name != b[i].name || a[i].score != b[i].score) ok = false;
    for (int i = 1; i < N; ++i) if (a[i - 1].score < a[i].score) sorted = false;
    CHECK(ok && sorted);

    mergeSort(a.get(), N, SortKey::Name);
    CHECK(binarySearchName(a.get(), N, "P7") == linearSearchName(a.get(), N, "p7") ||
          toLowerStr(a[binarySearchName(a.get(), N, "P7")].name) == "p7");
    CHECK(binarySearchName(a.get(), N, "p7") >= 0);
    CHECK(binarySearchName(a.get(), N, "nobody") == -1);
    CHECK(linearSearchName(a.get(), N, "nobody") == -1);
    CHECK(binarySearchName(a.get(), 0, "x") == -1);       // empty array

    std::remove(file);
}
}  // namespace tests_detail

inline void runAllTests() {
    g_testPass = g_testFail = 0;
    std::cout << "\n=== Running tests ===\n";
    tests_detail::testDataStructures();
    tests_detail::testMazeSizes();
    tests_detail::testGenerators();
    tests_detail::testLoops();
    tests_detail::testUnreachable();
    tests_detail::testMovement();
    tests_detail::testScoring();
    tests_detail::testLeaderboard();
    std::cout << "\nResult: " << g_testPass << " passed, " << g_testFail << " failed.\n";
}
