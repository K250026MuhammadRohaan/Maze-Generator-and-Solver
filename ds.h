// ds.h - Custom data structures (no STL containers): Stack, Queue, UnionFind
#pragma once
#include <stdexcept>

// ---------------------------------------------------------------- Stack ----
template <typename T>
class Stack {
    T* data_;
    int cap_;
    int size_;

    void grow() {
        int nc = cap_ * 2;
        T* nd = new T[nc];
        for (int i = 0; i < size_; ++i) nd[i] = data_[i];
        delete[] data_;
        data_ = nd;
        cap_ = nc;
    }

public:
    explicit Stack(int cap = 16) : data_(nullptr), cap_(cap > 0 ? cap : 16), size_(0) {
        data_ = new T[cap_];
    }
    ~Stack() { delete[] data_; }
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const T& v) {
        if (size_ == cap_) grow();
        data_[size_++] = v;
    }
    T pop() {
        if (size_ == 0) throw std::underflow_error("Stack::pop on empty stack");
        return data_[--size_];
    }
    T& top() {
        if (size_ == 0) throw std::underflow_error("Stack::top on empty stack");
        return data_[size_ - 1];
    }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }
    void clear() { size_ = 0; }
};

// ---------------------------------------------------------------- Queue ----
// Circular buffer that doubles its capacity when full.
template <typename T>
class Queue {
    T* data_;
    int cap_;
    int head_;
    int size_;

    void grow() {
        int nc = cap_ * 2;
        T* nd = new T[nc];
        for (int i = 0; i < size_; ++i) nd[i] = data_[(head_ + i) % cap_];
        delete[] data_;
        data_ = nd;
        cap_ = nc;
        head_ = 0;
    }

public:
    explicit Queue(int cap = 16) : data_(nullptr), cap_(cap > 0 ? cap : 16), head_(0), size_(0) {
        data_ = new T[cap_];
    }
    ~Queue() { delete[] data_; }
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void push(const T& v) {
        if (size_ == cap_) grow();
        data_[(head_ + size_) % cap_] = v;
        ++size_;
    }
    T pop() {
        if (size_ == 0) throw std::underflow_error("Queue::pop on empty queue");
        T v = data_[head_];
        head_ = (head_ + 1) % cap_;
        --size_;
        return v;
    }
    T& front() {
        if (size_ == 0) throw std::underflow_error("Queue::front on empty queue");
        return data_[head_];
    }
    bool empty() const { return size_ == 0; }
    int size() const { return size_; }
    void clear() { head_ = 0; size_ = 0; }
};

// ------------------------------------------------------------ UnionFind ----
// Disjoint-set union with path compression and union by rank.
class UnionFind {
    int* parent_;
    int* rank_;
    int n_;
    int sets_;

public:
    explicit UnionFind(int n) : parent_(nullptr), rank_(nullptr), n_(n), sets_(n) {
        if (n < 0) throw std::invalid_argument("UnionFind size must be >= 0");
        parent_ = new int[n > 0 ? n : 1];
        rank_ = new int[n > 0 ? n : 1];
        for (int i = 0; i < n; ++i) { parent_[i] = i; rank_[i] = 0; }
    }
    ~UnionFind() { delete[] parent_; delete[] rank_; }
    UnionFind(const UnionFind&) = delete;
    UnionFind& operator=(const UnionFind&) = delete;

    int find(int x) {
        if (x < 0 || x >= n_) throw std::out_of_range("UnionFind::find index");
        int root = x;
        while (parent_[root] != root) root = parent_[root];
        while (parent_[x] != root) {          // path compression
            int next = parent_[x];
            parent_[x] = root;
            x = next;
        }
        return root;
    }
    // Returns true if a and b were in different sets (and are now merged).
    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank_[ra] < rank_[rb]) { int t = ra; ra = rb; rb = t; }
        parent_[rb] = ra;                     // union by rank
        if (rank_[ra] == rank_[rb]) ++rank_[ra];
        --sets_;
        return true;
    }
    bool connected(int a, int b) { return find(a) == find(b); }
    int sets() const { return sets_; }
};
