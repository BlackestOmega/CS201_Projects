/**
 * @file BTree.h
 * @author your name (you@domain.com)
 * @brief Generic in-memory B-tree supporting insert, search and in-order traversal.
 *
 * 
 */
#pragma once
// BTree.h — simple B-tree

#include <cstddef>
#include <functional>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <vector>

/**
 * @brief A B-tree that stores unique keys of type T in sorted order.
 * 
 * @tparam T Key/record type.
 */
template <typename T>
class BTree {
    /**
     * @brief One node of the tree.
     * 
     */
    struct Node {
        std::vector<T> keys;
        std::vector<Node*> children;
        bool leaf;
        /**
         * @brief Creates an empty node.
         * 
         * @param isLeaf 
         */
        explicit Node(bool isLeaf) : leaf(isLeaf) {}
    };

    Node* root_ = nullptr;
    int t_;
    std::size_t count_ = 0;

    /**
     * @brief Recursively searches the subtree rooted at x for key k.
     * 
     * @param x Root of the subtree to search; may be nullptr.
     * @param k Key to look for
     * @return Pointer to the matching key inside the tree, or nullptr if not found.
     */
    const T* search(const Node* x, const T& k) const;

    /**
 * @brief Splits the full child x->children[i] into two nodes of t-1 keys each.
 *
 * The child's median key moves up into x at index i, and the new right-hand node
 * is inserted into x->children at index i+1.
 *
 * @param x Parent node; must not be full.
 * @param i Index of the full child within x->children.
 * @pre x->children[i] holds exactly 2t-1 keys.
 */
    void splitChild(Node* x, int i);

    /**
 * @brief Inserts k into the subtree rooted at x, splitting full children on the way down.
 * @param x Root of the subtree; must not be full.
 * @param k Key to insert; must not already be in the tree.
 * @pre x holds fewer than 2t-1 keys.
 */
    void insertNonFull(Node* x, const T& k);

    /**
 * @brief Recursively visits every key in the subtree rooted at x in sorted order.
 * @param x Root of the subtree; may be nullptr.
 * @param visit Function called once for each key.
 */
    void inorder(const Node* x, const std::function<void(const T&)>& visit) const;
    
    
/**
 * @brief Frees x and every node below it (children before parent).
 * @param x Root of the subtree to free; nullptr is allowed and does nothing.
 */
    void destroy(Node* x);

public:
    /**
    * @brief Creates an empty B-tree.
    * @param minDegree Minimum degree t; each full node holds 2t-1 keys.
    * @throws std::invalid_argument if minDegree is less than 2.
    */
    explicit BTree(int minDegree);
    
/**
 * @brief Frees every node in the tree.
 */
    ~BTree();
    BTree(const BTree&) = delete;
    BTree& operator=(const BTree&) = delete;

    /**
 * @brief Inserts a key into the tree.
 * @param k Key to insert.
 * @return true if inserted; false if an equal key was already present.
 */
    bool insert(const T& k);                  
   
    /**
 * @brief Looks up a key.
 * @param k Key to search for (only its key fields need to be set).
 * @return Pointer to the stored key, or nullptr if not found.
 * @warning The pointer can become invalid after the next insert, because splits
 *          move keys between nodes. Copy the value if you need to keep it.
 */
    const T* find(const T& k) const;  
    /**
 * @brief Calls visit on every key in ascending order.
 * @param visit Function called once per key.
 */       
    void forEach(const std::function<void(const T&)>& visit) const;  // sorted order
   
    /**
 * @brief Number of keys stored in the tree.
 * @return Key count.
 */
    std::size_t size() const { return count_; }
   
    /**
 * @brief Whether the tree holds no keys.
 * @return true if the tree is empty.
 */
    bool empty() const { return root_ == nullptr; }

    /**
 * @brief Prints the tree one level per line for debugging, e.g. "[2] [6 | 8]".
 * @param os Stream to print to (e.g. std::cout).
 */
    void printLevels(std::ostream& os) const; // debugging: one line per level
};

// ---------------------------------------------------------------------------

template <typename T>
BTree<T>::BTree(int minDegree) : t_(minDegree) {
    if (minDegree < 2){
        throw std::invalid_argument("minDegree must be >= 2");
    }
}

template <typename T>
BTree<T>::~BTree() { destroy(root_); }

template <typename T>
void BTree<T>::destroy(Node* x) {
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
        os << "[";
        for (std::size_t j = 0; j < node->keys.size(); j++) {
            if (j > 0) os << " | ";
            os << node->keys[j];
        }
        os << "] ";
        if (!node->leaf) {
            for (const Node* c : node->children) {
                q.push({c, level + 1});
            }
        }
    }

    os << "\n";
}
