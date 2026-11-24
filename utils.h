// utils.h - small helpers used by every module:
//   * random numbers   * text (char array) helpers   * keyboard input
// Only the plain C headers below are used (no STL).
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int LINE_SIZE = 128;   // size of the char arrays used to read one line

// ------------------------------------------------------------- random ----
inline void seedRandom() { srand((unsigned)time(NULL)); }   // call once at start

inline int randInt(int n) { return rand() % n; }            // 0 .. n-1 (n >= 1)

// Fisher-Yates shuffle: puts the n numbers of a[] in random order. O(n).
inline void shuffleArray(int* a, int n) {
    for (int i = n - 1; i > 0; --i) {
        int j = randInt(i + 1);
        int temp = a[i];
        a[i] = a[j];
        a[j] = temp;
    }
}

// --------------------------------------------------------------- text ----
inline char toUpperChar(char ch) { return (ch >= 'a' && ch <= 'z') ? (char)(ch - 'a' + 'A') : ch; }
inline char toLowerChar(char ch) { return (ch >= 'A' && ch <= 'Z') ? (char)(ch - 'A' + 'a') : ch; }
inline bool isSpaceChar(char ch) { return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n'; }

inline int textLength(const char* s) {
    int n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

// Copies src into dest (which holds destSize chars) and always adds the final '\0'.
inline void copyText(char* dest, const char* src, int destSize) {
    int i = 0;
    while (src[i] != '\0' && i < destSize - 1) {
        dest[i] = src[i];
        ++i;
    }
    dest[i] = '\0';
}

// Ignores upper/lower case. Returns < 0 if a comes first, 0 if equal, > 0 if b comes first.
inline int compareNoCase(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && toLowerChar(a[i]) == toLowerChar(b[i])) ++i;
    return (unsigned char)toLowerChar(a[i]) - (unsigned char)toLowerChar(b[i]);
}

inline bool isBlank(const char* s) {
    for (int i = 0; s[i] != '\0'; ++i)
        if (!isSpaceChar(s[i])) return false;
    return true;
}

// Cuts the '\n' and '\r' off the end of a line.
inline void removeLineEnd(char* s) {
    int n = textLength(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

// Turns text like " 42 " into the number 42. Returns false if it is not a number.
inline bool parseInt(const char* s, int& value) {
    int i = 0, sign = 1, digits = 0, v = 0;
    while (isSpaceChar(s[i])) ++i;
    if (s[i] == '-') { sign = -1; ++i; }
    else if (s[i] == '+') ++i;
    while (s[i] >= '0' && s[i] <= '9') {
        v = v * 10 + (s[i] - '0');
        ++digits;
        ++i;
        if (v > 1000000) return false;       // far too big (also avoids overflow)
    }
    while (isSpaceChar(s[i])) ++i;
    if (digits == 0 || s[i] != '\0') return false;
    value = sign * v;
    return true;
}

// -------------------------------------------------------------- input ----
// Shows the prompt and reads one line into buf. Returns false when input ends (EOF).
inline bool readLine(const char* prompt, char* buf, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return false;
    }
    int n = textLength(buf);
    if (n > 0 && buf[n - 1] != '\n') {       // line was longer than buf: throw the rest away
        int ch = getchar();
        while (ch != '\n' && ch != EOF) ch = getchar();
    }
    removeLineEnd(buf);
    return true;
}

// Keeps asking until the user types a whole number from lo to hi.
inline int readInt(const char* prompt, int lo, int hi) {
    char line[LINE_SIZE];
    int value = 0;
    for (;;) {
        if (!readLine(prompt, line, LINE_SIZE)) {
            printf("\nInput closed. Exiting.\n");
            exit(0);
        }
        if (parseInt(line, value) && value >= lo && value <= hi) return value;
        printf("  Please enter a number from %d to %d.\n", lo, hi);
    }
}

inline bool readYesNo(const char* prompt) {
    char line[LINE_SIZE];
    for (;;) {
        printf("%s (y/n): ", prompt);
        fflush(stdout);
        if (fgets(line, LINE_SIZE, stdin) == NULL) {
            printf("\nInput closed. Exiting.\n");
            exit(0);
        }
        char first = toLowerChar(line[0]);
        if (first == 'y') return true;
        if (first == 'n') return false;
    }
}

inline void pauseForEnter() {
    char line[LINE_SIZE];
    readLine("\nPress Enter to continue...", line, LINE_SIZE);
}

inline void clearScreen() {
#ifdef _WIN32
    if (system("cls") != 0) { }
#else
    if (system("clear") != 0) { }
#endif
}
