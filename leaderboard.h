// leaderboard.h - Module 2 (part 2): file-based leaderboard with sorting
//                 (insertion sort, merge sort) and searching (linear, binary).
#pragma once
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

struct Entry {
    std::string name;
    int score = 0;
    int moves = 0;
    int seconds = 0;
};

enum class SortKey { Score, Name };

inline std::string toLowerStr(std::string s) {
    for (char& ch : s) ch = (char)std::tolower((unsigned char)ch);
    return s;
}

// true if a should be placed before b
inline bool entryBefore(const Entry& a, const Entry& b, SortKey k) {
    if (k == SortKey::Name) {
        std::string x = toLowerStr(a.name), y = toLowerStr(b.name);
        if (x != y) return x < y;
        return a.score > b.score;
    }
    if (a.score != b.score) return a.score > b.score;         // higher score first
    if (a.seconds != b.seconds) return a.seconds < b.seconds; // then faster
    return a.name < b.name;
}

// O(k^2) - fine for a handful of records
inline void insertionSort(Entry* a, int n, SortKey k) {
    for (int i = 1; i < n; ++i) {
        Entry key = a[i];
        int j = i - 1;
        while (j >= 0 && entryBefore(key, a[j], k)) { a[j + 1] = a[j]; --j; }
        a[j + 1] = key;
    }
}

inline void mergeSortRec(Entry* a, Entry* tmp, int lo, int hi, SortKey k) {  // [lo, hi)
    if (hi - lo < 2) return;
    int mid = lo + (hi - lo) / 2;
    mergeSortRec(a, tmp, lo, mid, k);
    mergeSortRec(a, tmp, mid, hi, k);
    int i = lo, j = mid, t = lo;
    while (i < mid && j < hi) tmp[t++] = entryBefore(a[j], a[i], k) ? a[j++] : a[i++];
    while (i < mid) tmp[t++] = a[i++];
    while (j < hi) tmp[t++] = a[j++];
    for (int x = lo; x < hi; ++x) a[x] = tmp[x];
}
// O(k log k)
inline void mergeSort(Entry* a, int n, SortKey k) {
    if (n < 2) return;
    std::unique_ptr<Entry[]> tmp(new Entry[n]);
    mergeSortRec(a, tmp.get(), 0, n, k);
}

// O(k): works on any order. Returns index or -1.
inline int linearSearchName(const Entry* a, int n, const std::string& name) {
    std::string t = toLowerStr(name);
    for (int i = 0; i < n; ++i) if (toLowerStr(a[i].name) == t) return i;
    return -1;
}
// O(log k): array must be sorted by name (SortKey::Name). Returns index or -1.
inline int binarySearchName(const Entry* a, int n, const std::string& name) {
    std::string t = toLowerStr(name);
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        std::string v = toLowerStr(a[mid].name);
        if (v == t) return mid;
        if (v < t) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

class Leaderboard {
public:
    static constexpr int MAX_RECORDS = 64;   // read limit when loading
    static constexpr int TOP_N = 5;          // records kept

private:
    std::string path_;
    Entry e_[MAX_RECORDS];
    int n_ = 0;
    int skipped_ = 0;

    static std::string sanitize(std::string s) {
        for (char& ch : s) if (std::isspace((unsigned char)ch)) ch = '_';
        if (s.size() > 15) s.resize(15);
        if (s.empty()) s = "Player";
        return s;
    }

public:
    explicit Leaderboard(const std::string& path = "scores.txt") : path_(path) {}

    // Returns false if the file does not exist (board is then empty).
    // Malformed lines are skipped and counted; only the top five are kept.
    bool load() {
        n_ = 0; skipped_ = 0;
        std::ifstream in(path_);
        if (!in) return false;
        std::string line;
        while (n_ < MAX_RECORDS && std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.find_first_not_of(" \t") == std::string::npos) continue;  // blank
            std::istringstream ss(line);
            Entry e; std::string extra;
            if (!(ss >> e.name >> e.score >> e.moves >> e.seconds) || (ss >> extra) ||
                e.score < 0 || e.moves < 0 || e.seconds < 0) { ++skipped_; continue; }
            e_[n_++] = e;
        }
        insertionSort(e_, n_, SortKey::Score);
        if (n_ > TOP_N) n_ = TOP_N;
        return true;
    }

    bool save() const {
        std::ofstream out(path_, std::ios::trunc);
        if (!out) return false;
        for (int i = 0; i < n_; ++i)
            out << e_[i].name << ' ' << e_[i].score << ' ' << e_[i].moves << ' ' << e_[i].seconds << '\n';
        return (bool)out;
    }

    // Insert, re-sort, keep the top five.
    void add(Entry e) {
        e.name = sanitize(e.name);
        if (n_ < MAX_RECORDS) e_[n_++] = e;
        insertionSort(e_, n_, SortKey::Score);
        if (n_ > TOP_N) n_ = TOP_N;
    }

    int size() const { return n_; }
    int skippedLines() const { return skipped_; }
    const Entry& at(int i) const { return e_[i]; }
    const std::string& path() const { return path_; }
    void clear() { n_ = 0; skipped_ = 0; }

    // Copy of the records for sorting/searching demos.
    std::unique_ptr<Entry[]> copyRecords() const {
        std::unique_ptr<Entry[]> c(new Entry[n_ > 0 ? n_ : 1]);
        for (int i = 0; i < n_; ++i) c[i] = e_[i];
        return c;
    }

    static void printTable(const Entry* a, int n) {
        if (n == 0) { std::cout << "  (no scores yet)\n"; return; }
        std::cout << "  " << std::left << std::setw(5) << "Rank" << std::setw(17) << "Name"
                  << std::right << std::setw(8) << "Score" << std::setw(8) << "Moves"
                  << std::setw(8) << "Time(s)" << '\n';
        for (int i = 0; i < n; ++i)
            std::cout << "  " << std::left << std::setw(5) << (i + 1) << std::setw(17) << a[i].name
                      << std::right << std::setw(8) << a[i].score << std::setw(8) << a[i].moves
                      << std::setw(8) << a[i].seconds << '\n';
    }
    void print() const { printTable(e_, n_); }
};
