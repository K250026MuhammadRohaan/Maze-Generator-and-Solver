// maze.h - Module 1: Maze core (grid, bounds/collision), DFS generation,
//          loop addition, console display and player movement.
#pragma once
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include "ds.h"

struct Cell { int r, c; };

// ------------------------------------------------------------ randomness ----
inline std::mt19937& rng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}
inline void seedRng(unsigned s) { rng().seed(s); }
inline int randInt(int n) {                    // uniform in [0, n)
    return std::uniform_int_distribution<int>(0, n - 1)(rng());
}
template <typename T>
void shuffleArray(T* a, int n) {               // Fisher-Yates, O(n)
    for (int i = n - 1; i > 0; --i) std::swap(a[i], a[randInt(i + 1)]);
}

// ----------------------------------------------------------------- Maze ----
// Grid of size rows x cols (both odd). Rooms sit on odd/odd coordinates;
// the cells between two rooms are "walls" that may be opened.
class Maze {
    int rows_, cols_;
    char* g_;   // 1 = wall, 0 = open

public:
    static bool validSize(int r, int c) {
        return r >= 5 && c >= 5 && r <= 101 && c <= 101 && (r % 2 == 1) && (c % 2 == 1);
    }

    Maze(int r, int c) : rows_(r), cols_(c), g_(nullptr) {
        if (!validSize(r, c))
            throw std::invalid_argument("Maze size must be odd numbers between 5 and 101");
        g_ = new char[r * c];
        fillWalls();
    }
    Maze(const Maze& o) : rows_(o.rows_), cols_(o.cols_), g_(new char[o.rows_ * o.cols_]) {
        std::copy(o.g_, o.g_ + rows_ * cols_, g_);
    }
    Maze(Maze&& o) noexcept : rows_(o.rows_), cols_(o.cols_), g_(o.g_) { o.g_ = nullptr; }
    Maze& operator=(Maze o) noexcept {
        std::swap(rows_, o.rows_); std::swap(cols_, o.cols_); std::swap(g_, o.g_);
        return *this;
    }
    ~Maze() { delete[] g_; }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int rooms() const { return ((rows_ - 1) / 2) * ((cols_ - 1) / 2); }
    Cell start() const { return Cell{1, 1}; }
    Cell exit() const { return Cell{rows_ - 2, cols_ - 2}; }

    bool inBounds(int r, int c) const { return r >= 0 && r < rows_ && c >= 0 && c < cols_; }
    bool isWall(int r, int c) const { return !inBounds(r, c) || g_[r * cols_ + c] == 1; }
    bool isOpen(int r, int c) const { return !isWall(r, c); }
    void setOpen(int r, int c) { if (inBounds(r, c)) g_[r * cols_ + c] = 0; }
    void setWall(int r, int c) { if (inBounds(r, c)) g_[r * cols_ + c] = 1; }
    void fillWalls() { std::fill(g_, g_ + rows_ * cols_, (char)1); }

    int openCount() const {
        int n = 0;
        for (int i = 0; i < rows_ * cols_; ++i) if (g_[i] == 0) ++n;
        return n;
    }
    // Number of opened passages between rooms. A perfect maze has rooms()-1.
    int openConnectors() const {
        int n = 0;
        for (int r = 1; r < rows_ - 1; ++r)
            for (int c = 1; c < cols_ - 1; ++c)
                if (((r + c) & 1) == 1 && g_[r * cols_ + c] == 0) ++n;
        return n;
    }

    // mark (optional): array of rows*cols chars, non-zero = show path dot.
    void display(const Cell* player = nullptr, const char* mark = nullptr) const {
        std::string line;
        Cell ex = exit();
        for (int r = 0; r < rows_; ++r) {
            line.clear();
            for (int c = 0; c < cols_; ++c) {
                if (player && player->r == r && player->c == c) line += "P ";
                else if (r == 1 && c == 1) line += "S ";
                else if (r == ex.r && c == ex.c) line += "E ";
                else if (isWall(r, c)) line += "##";
                else if (mark && mark[r * cols_ + c]) line += ". ";
                else line += "  ";
            }
            std::cout << line << '\n';
        }
    }
};

// ------------------------------------------------- DFS (backtracking) gen ----
// Iterative recursive-backtracker using the custom Stack. O(n) time/space.
inline void generateDFS(Maze& m) {
    m.fillWalls();
    static const int dr[4] = {-2, 0, 2, 0};
    static const int dc[4] = {0, 2, 0, -2};
    Stack<Cell> st(64);
    Cell s = m.start();
    m.setOpen(s.r, s.c);
    st.push(s);
    while (!st.empty()) {
        Cell cur = st.top();
        int cand[4], k = 0;
        for (int d = 0; d < 4; ++d) {
            int nr = cur.r + dr[d], nc = cur.c + dc[d];
            if (m.inBounds(nr, nc) && m.isWall(nr, nc)) cand[k++] = d;
        }
        if (k == 0) { st.pop(); continue; }       // dead end: backtrack
        int d = cand[randInt(k)];
        m.setOpen(cur.r + dr[d] / 2, cur.c + dc[d] / 2);
        Cell nx{cur.r + dr[d], cur.c + dc[d]};
        m.setOpen(nx.r, nx.c);
        st.push(nx);
    }
}

// Opens up to `count` interior walls that separate two open rooms, creating
// alternative routes. Returns how many were opened. O(n).
inline int addLoops(Maze& m, int count) {
    if (count <= 0) return 0;
    std::unique_ptr<Cell[]> cand(new Cell[m.rows() * m.cols()]);
    int k = 0;
    for (int r = 1; r < m.rows() - 1; ++r) {
        for (int c = 1; c < m.cols() - 1; ++c) {
            if (!m.isWall(r, c)) continue;
            bool horiz = (r % 2 == 1) && (c % 2 == 0) && m.isOpen(r, c - 1) && m.isOpen(r, c + 1);
            bool vert = (r % 2 == 0) && (c % 2 == 1) && m.isOpen(r - 1, c) && m.isOpen(r + 1, c);
            if (horiz || vert) cand[k++] = Cell{r, c};
        }
    }
    shuffleArray(cand.get(), k);
    int opened = std::min(count, k);
    for (int i = 0; i < opened; ++i) m.setOpen(cand[i].r, cand[i].c);
    return opened;
}

// ----------------------------------------------------------- movement ----
// key: W/A/S/D (any case). Returns true only if the player actually moved.
inline bool tryMove(const Maze& m, Cell& p, char key) {
    int dr = 0, dc = 0;
    switch (std::toupper((unsigned char)key)) {
        case 'W': dr = -1; break;
        case 'S': dr = 1; break;
        case 'A': dc = -1; break;
        case 'D': dc = 1; break;
        default: return false;
    }
    int nr = p.r + dr, nc = p.c + dc;
    if (m.isWall(nr, nc)) return false;           // wall or outside the grid
    p.r = nr; p.c = nc;
    return true;
}
