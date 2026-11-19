#pragma once
// BTree.h — simple B-tree

#include <cstddef>
#include <functional>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <vector>

template <typename T>
class BTree {
    struct Node {
        std::vector<T> keys;
        std::vector<Node*> children;
        bool leaf;
        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    Node* root_ = nullptr;
    int t_;
    std::size_t count_ = 0;

    const T* search(const Node* x, const T& k) const;
    void splitChild(Node* x, int i);
    void insertNonFull(Node* x, const T& k);
    void inorder(const Node* x, const std::function<void(const T&)>& visit) const;
    void destroy(Node* x);

public:
    explicit BTree(int minDegree);
    ~BTree();
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;

    bool insert(const T& k);                  // false if k is already in the tree
    const T* find(const T& k) const;          // nullptr if not found
    void forEach(const std::function<void(const T&)>& visit) const;  // sorted order
    std::size_t size() const { return count_; }
    bool empty() const { return root_ == nullptr; }
    void printLevels(std::ostream& os) const; // debugging: one line per level
};

// ---------------------------------------------------------------------------

template <typename T>
BTree<T>::BTree(int minDegree) : t_(minDegree) {
    // TODO: throw std::invalid_argument if minDegree < 2
    if (minDegree < 2){
        throw std::invalid_argument("minDegree must be >= 2");
    }
}

template <typename T>
BTree<T>::~BTree() { destroy(root_); }

template <typename T>
void BTree<T>::destroy(Node* x) {
    // TODO: null -> return; destroy every child; then delete x
    if(x == nullptr) return;
    for(Node* c : x->children){
        destroy(c);
    }
    delete x;
}

template <typename T>
const T* BTree<T>::find(const T& k) const { return search(root_, k); }

template <typename T>
const T* BTree<T>::search(const Node* x, const T& k) const {
    if(x == nullptr) return nullptr;
    int idx = 0;
    while (idx < (int)x->keys.size() && x->keys[idx] < k) {idx++;}
    if (idx < (int)x->keys.size() && x->keys[idx] == k) return &x->keys[idx];
    else if(x->leaf) return nullptr;
    return search(x->children[idx], k);
}

template <typename T>
bool BTree<T>::insert(const T& k) {
    if(find(k) != nullptr) return false;

    // if tree is empty
    if(root_ == nullptr){
        root_ = new Node(true);
        root_->keys.push_back(k);
        ++count_;
        return true;
    }

    // if node full
    if ((int)root_->keys.size() == 2 * t_ - 1) {
        Node* s = new Node(false);      
        s->children.push_back(root_);   
        root_ = s;
        splitChild(s, 0);              
    }

    insertNonFull(root_, k);
    ++count_;
    return true;
}

template <typename T>
void BTree<T>::splitChild(Node* x, int idx) {
    Node* full = x->children[idx];
    Node* buffer = new Node(full->leaf);
    T median = full->keys[t_ - 1];

    buffer->keys.assign(full->keys.begin() + t_, full->keys.end());

    if(!full->leaf){
        buffer->children.assign(full->children.begin() + t_, full->children.end());
        full->children.resize(t_);
        
    }
    

    full->keys.resize(t_ - 1);
    x->children.insert(x->children.begin() + idx +1, buffer);
    x->keys.insert(x->keys.begin() + idx, median);
    

}

template <typename T>
void BTree<T>::insertNonFull(Node* x, const T& k) {
    int idx = 0;
    while (idx < (int)x->keys.size() && x->keys[idx] < k) {idx++;}
    if(x->leaf){
        x->keys.insert(x->keys.begin() + idx, k);
    }
    else {
        if(x->children[idx]->keys.size() == 2*t_ - 1){
            splitChild(x, idx);
            if(x->keys[idx] < k){
                idx++;
            }
        }
        insertNonFull(x->children[idx], k);
    }
   
}

template <typename T>
void BTree<T>::forEach(const std::function<void(const T&)>& visit) const {
    inorder(root_, visit);
}

template <typename T>
void BTree<T>::inorder(const Node* x, const std::function<void(const T&)>& visit) const {
    if(x == nullptr) return;
    for(int i = 0; i < x->keys.size(); i++){
        if (!x->leaf) {
            inorder(x->children[i], visit);
        }
        visit(x->keys[i]);
    }
    if (!x->leaf) {
        inorder(x->children.back(), visit);
    }
}

template <typename T>
void BTree<T>::printLevels(std::ostream& os) const {

    if (root_ == nullptr) {
        os << "empty\n";
        return;
    }

    std::queue<std::pair<const Node*, int>> q;   
    q.push({root_, 0});  
    int current_level = 0;

    while (!q.empty()) {
        auto [node, level] = q.front();
        q.pop();
        if (level != current_level) {
            os << "\n";
            current_level = level;
        }
        if (!node->leaf) {
            for (const Node* c : node->children) {
                q.push({c, level + 1});
            }
        }
    }

    os << "\n";
}
