// ds.h - Our own data structures (no STL): Stack, Queue and UnionFind.
// Stack and Queue store plain ints. In this project an int is usually a cell
// number of the maze: cell = row * cols + col.
#pragma once
#include <stdlib.h>   // NULL

// ---------------------------------------------------------------- Stack ----
// Last in, first out. Stored in an array that doubles in size when it is full.
class Stack {
    int* data_;
    int cap_;     // how many ints fit in data_
    int size_;    // how many ints are stored

    Stack(const Stack&);               // private and never defined:
    Stack& operator=(const Stack&);    // a Stack cannot be copied

    void grow() {                      // make a bigger array and copy everything over
        int newCap = cap_ * 2;
        int* bigger = new int[newCap];
        for (int i = 0; i < size_; ++i) bigger[i] = data_[i];
        delete[] data_;
        data_ = bigger;
        cap_ = newCap;
    }

public:
    explicit Stack(int cap = 16) : data_(NULL), cap_(cap > 0 ? cap : 16), size_(0) {
        data_ = new int[cap_];
    }
    ~Stack() { delete[] data_; }

    void push(int v) {
        if (size_ == cap_) grow();
        data_[size_++] = v;
    }
    int pop() {                        // returns -1 if the stack is empty
        if (size_ == 0) return -1;
        return data_[--size_];
    }
    int top() const {                  // looks at the top without removing it (-1 if empty)
        if (size_ == 0) return -1;
        return data_[size_ - 1];
    }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }
};

// ---------------------------------------------------------------- Queue ----
// First in, first out. A circular array: when the end is reached we wrap
// around to index 0, and the array doubles in size when it is full.
class Queue {
    int* data_;
    int cap_;
    int head_;    // index of the front item
    int size_;

    Queue(const Queue&);
    Queue& operator=(const Queue&);

    void grow() {
        int newCap = cap_ * 2;
        int* bigger = new int[newCap];
        for (int i = 0; i < size_; ++i) bigger[i] = data_[(head_ + i) % cap_];
        delete[] data_;
        data_ = bigger;
        cap_ = newCap;
        head_ = 0;
    }

public:
    explicit Queue(int cap = 16) : data_(NULL), cap_(cap > 0 ? cap : 16), head_(0), size_(0) {
        data_ = new int[cap_];
    }
    ~Queue() { delete[] data_; }

    void push(int v) {
        if (size_ == cap_) grow();
        data_[(head_ + size_) % cap_] = v;
        ++size_;
    }
    int pop() {                        // returns -1 if the queue is empty
        if (size_ == 0) return -1;
        int v = data_[head_];
        head_ = (head_ + 1) % cap_;
        --size_;
        return v;
    }
    int front() const {                // looks at the front without removing it (-1 if empty)
        if (size_ == 0) return -1;
        return data_[head_];
    }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }
};

// ------------------------------------------------------------ UnionFind ----
// Keeps track of which items are in the same group.
//   find(x)   - the "leader" of x's group (path compression makes it fast)
//   unite(a,b) - joins two groups (the shorter tree goes under the taller one)
// Items are the numbers 0 .. n-1.
class UnionFind {
    int* parent_;
    int* rank_;
    int sets_;     // how many separate groups exist

    UnionFind(const UnionFind&);
    UnionFind& operator=(const UnionFind&);

public:
    explicit UnionFind(int n) : parent_(NULL), rank_(NULL), sets_(n) {
        parent_ = new int[n > 0 ? n : 1];
        rank_ = new int[n > 0 ? n : 1];
        for (int i = 0; i < n; ++i) {
            parent_[i] = i;            // everyone starts alone, as their own leader
            rank_[i] = 0;
        }
    }
    ~UnionFind() {
        delete[] parent_;
        delete[] rank_;
    }

    int find(int x) {
        int root = x;
        while (parent_[root] != root) root = parent_[root];
        while (parent_[x] != root) {   // path compression: point everyone straight at the leader
            int next = parent_[x];
            parent_[x] = root;
            x = next;
        }
        return root;
    }

    // Returns true if a and b were in different groups (they are merged now).
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank_[ra] < rank_[rb]) { int t = ra; ra = rb; rb = t; }
        parent_[rb] = ra;
        if (rank_[ra] == rank_[rb]) ++rank_[ra];
        --sets_;
        return true;
    }

    bool connected(int a, int b) { return find(a) == find(b); }
    int sets() const { return sets_; }
};
