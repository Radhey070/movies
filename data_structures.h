#pragma once
#include "models.h"

// These classes have concrete types. No templates or callback functions.
#include <stdexcept>

// 1. SINGLY LINKED LIST: traverse the movie roster during candidate generation.
struct MovieNode {
    Movie movie;
    MovieNode* next;
};
struct MovieList {
    MovieNode* head = nullptr;
    MovieNode* tail = nullptr;
    void append(const Movie& movie) {
        MovieNode* node = new MovieNode{movie, nullptr};
        if (head == nullptr) head = node;
        else tail->next = node;
        tail = node;
    }
    ~MovieList() {
        while (head != nullptr) {
            MovieNode* old = head;
            head = head->next;
            delete old;
        }
    }
};

// 2. DOUBLY LINKED LIST: accepted shows, ordered by screen then start time.
struct ShowNode {
    Show show;
    ShowNode* prev;
    ShowNode* next;
};
struct ShowList {
    ShowNode* head = nullptr;
    ShowNode* tail = nullptr;
    void insertSorted(const Show& show) {
        ShowNode* node = new ShowNode{show, nullptr, nullptr};
        ShowNode* current = head;
        while (current != nullptr &&
               (current->show.screenId < show.screenId ||
               (current->show.screenId == show.screenId && current->show.startMinutes < show.startMinutes)))
            current = current->next;
        if (current == nullptr) { // append at the end
            node->prev = tail;
            if (tail != nullptr) tail->next = node;
            else head = node;
            tail = node;
        } else { // insert before current
            node->next = current;
            node->prev = current->prev;
            if (current->prev != nullptr) current->prev->next = node;
            else head = node;
            current->prev = node;
        }
    }
    std::vector<Show> toVector() const {
        std::vector<Show> shows;
        for (ShowNode* node = head; node != nullptr; node = node->next)
            shows.push_back(node->show);
        return shows;
    }
    ~ShowList() {
        while (head != nullptr) {
            ShowNode* old = head;
            head = head->next;
            delete old;
        }
    }
};

// 3. FIFO QUEUE: process already-ranked candidates from front to back.
struct QueueNode { Show show; QueueNode* next; };
struct ShowQueue {
    QueueNode* front = nullptr;
    QueueNode* rear = nullptr;
    bool empty() const { return front == nullptr; }
    void enqueue(const Show& show) {
        QueueNode* node = new QueueNode{show, nullptr};
        if (rear != nullptr) rear->next = node;
        else front = node;
        rear = node;
    }
    Show dequeue() {
        if (empty()) throw std::runtime_error("Queue is empty");
        QueueNode* old = front;
        Show answer = old->show;
        front = front->next;
        if (front == nullptr) rear = nullptr;
        delete old;
        return answer;
    }
    ~ShowQueue() { while (!empty()) dequeue(); }
};

// 4. CIRCULAR QUEUE: the same movie indexes repeat in the baseline.
struct CircularQueue {
    std::vector<int> items;
    int front = 0;
    CircularQueue(int movieCount) {
        for (int i = 0; i < movieCount; i++) items.push_back(i);
    }
    int nextMovie() {
        int answer = items[front];
        front = (front + 1) % items.size();
        return answer;
    }
};

// 5. LIFO STACK: print accepted decisions from newest to oldest.
struct DecisionStack {
    std::vector<Show> items;
    void push(const Show& show) { items.push_back(show); }
    bool empty() const { return items.empty(); }
    Show pop() {
        if (empty()) throw std::runtime_error("Stack is empty");
        Show answer = items.back();
        items.pop_back();
        return answer;
    }
};

// 6. BST: look up a movie's position using its ID. It is not balanced.
struct TreeNode { int id; int movieIndex; TreeNode* left; TreeNode* right; };
struct MovieBST {
    TreeNode* root = nullptr;
    TreeNode* insertNode(TreeNode* node, int id, int index) {
        if (node == nullptr) return new TreeNode{id,index,nullptr,nullptr};
        if (id < node->id) node->left = insertNode(node->left,id,index);
        else if (id > node->id) node->right = insertNode(node->right,id,index);
        return node;
    }
    void insert(int id, int index) { root = insertNode(root,id,index); }
    int find(int id) const {
        TreeNode* node = root;
        while (node != nullptr) {
            if (id == node->id) return node->movieIndex;
            if (id < node->id) node = node->left;
            else node = node->right;
        }
        return -1;
    }
    void clear(TreeNode* node) {
        if (node == nullptr) return;
        clear(node->left); clear(node->right); delete node;
    }
    ~MovieBST() { clear(root); }
};

