// leaderboard.h - Module 2 (part 2): file-based leaderboard with sorting
//                 (insertion sort, merge sort) and searching (linear, binary).
#pragma once
#include <stdio.h>
#include "utils.h"

const int NAME_SIZE = 16;       // 15 letters + the final '\0'
const int MAX_RECORDS = 64;     // most lines read from the file
const int TOP_N = 5;            // scores kept on the board

struct Entry {
    char name[NAME_SIZE];
    int score;
    int moves;
    int seconds;
    Entry() : score(0), moves(0), seconds(0) { name[0] = '\0'; }
};

enum SortKey { BY_SCORE, BY_NAME };

// true if a must be placed before b
inline bool entryBefore(const Entry& a, const Entry& b, SortKey key) {
    if (key == BY_NAME) {
        int cmp = compareNoCase(a.name, b.name);
        if (cmp != 0) return cmp < 0;
        return a.score > b.score;
    }
    if (a.score != b.score) return a.score > b.score;            // higher score first
    if (a.seconds != b.seconds) return a.seconds < b.seconds;    // then the faster one
    return compareNoCase(a.name, b.name) < 0;                    // then by name
}

// ------------------------------------------------------------ sorting ----
// Insertion sort: take each item and slide it left into its place.
// O(k^2), which is fine for a handful of records.
inline void insertionSort(Entry* a, int n, SortKey key) {
    for (int i = 1; i < n; ++i) {
        Entry item = a[i];
        int j = i - 1;
        while (j >= 0 && entryBefore(item, a[j], key)) {
            a[j + 1] = a[j];       // shift bigger items to the right
            --j;
        }
        a[j + 1] = item;
    }
}

// Sorts a[lo .. hi-1]: sort both halves, then merge them using tmp[].
inline void mergeSortPart(Entry* a, Entry* tmp, int lo, int hi, SortKey key) {
    if (hi - lo < 2) return;                 // 0 or 1 item is already sorted
    int mid = lo + (hi - lo) / 2;
    mergeSortPart(a, tmp, lo, mid, key);
    mergeSortPart(a, tmp, mid, hi, key);

    int i = lo, j = mid, t = lo;             // i: left half, j: right half
    while (i < mid && j < hi) {
        if (entryBefore(a[j], a[i], key)) tmp[t++] = a[j++];
        else tmp[t++] = a[i++];
    }
    while (i < mid) tmp[t++] = a[i++];
    while (j < hi) tmp[t++] = a[j++];
    for (int x = lo; x < hi; ++x) a[x] = tmp[x];
}

// Merge sort: O(k log k).
inline void mergeSort(Entry* a, int n, SortKey key) {
    if (n < 2) return;
    Entry* tmp = new Entry[n];
    mergeSortPart(a, tmp, 0, n, key);
    delete[] tmp;
}

// ---------------------------------------------------------- searching ----
// Linear search: check every item. O(k), works on any order. Returns index or -1.
inline int linearSearchName(const Entry* a, int n, const char* name) {
    for (int i = 0; i < n; ++i)
        if (compareNoCase(a[i].name, name) == 0) return i;
    return -1;
}

// Binary search: the array must be sorted by name (BY_NAME). Look at the
// middle, then throw away the half that cannot contain the name. O(log k).
inline int binarySearchName(const Entry* a, int n, const char* name) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int cmp = compareNoCase(a[mid].name, name);
        if (cmp == 0) return mid;
        if (cmp < 0) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// -------------------------------------------------------- Leaderboard ----
// File format: one player per line:  name score moves seconds
class Leaderboard {
    char path_[LINE_SIZE];
    Entry entries_[MAX_RECORDS];
    int n_;
    int skipped_;      // bad lines found by the last load()

    Leaderboard(const Leaderboard&);
    Leaderboard& operator=(const Leaderboard&);

public:
    explicit Leaderboard(const char* path = "scores.txt") : n_(0), skipped_(0) {
        copyText(path_, path, LINE_SIZE);
    }

    // Returns false if the file does not exist (the board is then empty).
    // Bad lines are skipped and counted. Only the top five are kept.
    bool load() {
        n_ = 0;
        skipped_ = 0;
        FILE* in = fopen(path_, "r");
        if (in == NULL) return false;
        char line[LINE_SIZE];
        while (n_ < MAX_RECORDS && fgets(line, LINE_SIZE, in) != NULL) {
            removeLineEnd(line);
            if (isBlank(line)) continue;
            Entry e;
            char extra;                 // catches anything left over after the 4 values
            int got = sscanf(line, "%15s %d %d %d %c", e.name, &e.score, &e.moves, &e.seconds, &extra);
            if (got != 4 || e.score < 0 || e.moves < 0 || e.seconds < 0) {
                ++skipped_;
                continue;
            }
            entries_[n_++] = e;
        }
        fclose(in);
        insertionSort(entries_, n_, BY_SCORE);
        if (n_ > TOP_N) n_ = TOP_N;
        return true;
    }

    bool save() const {
        FILE* out = fopen(path_, "w");
        if (out == NULL) return false;
        for (int i = 0; i < n_; ++i)
            fprintf(out, "%s %d %d %d\n", entries_[i].name, entries_[i].score,
                    entries_[i].moves, entries_[i].seconds);
        return fclose(out) == 0;
    }

    // Adds a score, sorts, and keeps only the top five.
    // The name is cleaned: spaces become '_', at most 15 letters, empty becomes "Player".
    void add(const char* name, int score, int moves, int seconds) {
        Entry e;
        copyText(e.name, name, NAME_SIZE);
        for (int i = 0; e.name[i] != '\0'; ++i)
            if (isSpaceChar(e.name[i])) e.name[i] = '_';
        if (e.name[0] == '\0') copyText(e.name, "Player", NAME_SIZE);
        e.score = score;
        e.moves = moves;
        e.seconds = seconds;

        if (n_ < MAX_RECORDS) entries_[n_++] = e;
        insertionSort(entries_, n_, BY_SCORE);
        if (n_ > TOP_N) n_ = TOP_N;
    }

    int size() const { return n_; }
    int skippedLines() const { return skipped_; }
    const Entry& at(int i) const { return entries_[i]; }
    const char* path() const { return path_; }

    // Copies the records into out[] (which must hold MAX_RECORDS) and returns how many.
    int copyTo(Entry* out) const {
        for (int i = 0; i < n_; ++i) out[i] = entries_[i];
        return n_;
    }

    static void printTable(const Entry* a, int n) {
        if (n == 0) {
            printf("  (no scores yet)\n");
            return;
        }
        printf("  %-5s%-17s%8s%8s%8s\n", "Rank", "Name", "Score", "Moves", "Time(s)");
        for (int i = 0; i < n; ++i)
            printf("  %-5d%-17s%8d%8d%8d\n", i + 1, a[i].name, a[i].score, a[i].moves, a[i].seconds);
    }
    void print() const { printTable(entries_, n_); }
};
