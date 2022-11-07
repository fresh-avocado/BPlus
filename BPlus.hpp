#ifndef BPLUS_HPP_
#define BPLUS_HPP_

#include <stdlib.h>
#include <string>

#ifndef CACHELINESIZE
#define CACHELINESIZE 64
#endif

#ifdef ALIGNED_MEM
#define ALLOC_MEM(type, num) \
  (type*)aligned_alloc(CACHELINESIZE, sizeof(type) * num)
#define ALLOC_OBJ(objInstance) new (1, CACHELINESIZE) objInstance
#else
#define ALLOC_MEM(type, num) (type*)malloc(sizeof(type) * num)
#define ALLOC_OBJ(objInstance) new objInstance
#endif

struct NodeFlags {
  enum : std::uint8_t {
    isRoot = 1 << 1,
    isLeaf = 1 << 2,
  };
};

template <int degree>
class BPlusTree;

template <int degree>
class Node {
  // TOOD: encapsular
 public:
  int* keys;
  Node<degree>** children;
  int numKeys = 0;
  uint8_t flags = 0;
  Node<degree>* next = nullptr;
  Node<degree>* prev = nullptr;

  Node() {
    keys = ALLOC_MEM(int, degree);
    children = ALLOC_MEM(Node<degree>*, degree + 1);
  }
  Node(uint8_t flags) {
    keys = ALLOC_MEM(int, degree);
    children = ALLOC_MEM(Node<degree>*, degree + 1);
    this->flags |= flags;
  }
  void killSelf() {
    free(keys);
    free(children);
  }
  friend BPlusTree<degree>;
};

template <int degree>
class BPlusTree {
  Node<degree>* root;
  int height = 0;

 public:
  BPlusTree() {
    root = ALLOC_OBJ(Node<degree>(NodeFlags::isLeaf | NodeFlags::isRoot));
  }
  std::string inorderString() const {
    std::string str = "";
    inorder(str);
    return str;
  }
  void clear() {
    clear(root);
    delete root;
  }
  int getHeight() const { return height; }
  std::string getRoot() const {
    std::string str = "[";
    int i;
    for (i = 0; i < root->numKeys - 1; i++) {
      str += std::to_string(root->keys[i]);
      str += "|";
    }
    str += std::to_string(root->keys[i]);
    str += "]";
    return str;
  }
  int search(int k) const { return search(root, k); }
  void insert(int k) { insert(nullptr, root, k, -1); }
  ~BPlusTree() noexcept { clear(); }

 private:
  void insert(Node<degree>* parent,
              Node<degree>* node,
              int k,
              int parentChildIndex) {
    int i = 0;
    while (i < node->numKeys && k >= node->keys[i]) {
      i++;
    }
    if (node->flags & NodeFlags::isLeaf) {
      for (int j = node->numKeys; j > i; j--) {
        node->keys[j] = node->keys[j - 1];
      }
      node->keys[i] = k;
      if (++node->numKeys == degree) {
        int splitPos = degree / 2;
        Node<degree>* newNode = ALLOC_OBJ(Node<degree>(NodeFlags::isLeaf));
        for (int j = splitPos; j < node->numKeys; j++) {
          newNode->keys[j - splitPos] = node->keys[j];
          newNode->numKeys++;
        }
        node->numKeys = splitPos;
        newNode->next = node->next;
        node->next = newNode;
        newNode->prev = node;
        if (node->flags & NodeFlags::isRoot) {
          node->flags &= ~NodeFlags::isRoot;
          Node<degree>* newRoot = ALLOC_OBJ(Node<degree>(NodeFlags::isRoot));
          newRoot->keys[0] = newNode->keys[0];
          newRoot->numKeys++;
          newRoot->children[0] = node;
          newRoot->children[1] = newNode;
          root = newRoot;
          height++;
        } else {
          for (int j = parent->numKeys; j > parentChildIndex; j--) {
            parent->keys[j] = parent->keys[j - 1];
            parent->children[j + 1] = parent->children[j];
          }
          parent->keys[parentChildIndex] = newNode->keys[0];
          parent->numKeys++;
          parent->children[parentChildIndex + 1] = newNode;
        }
      }
    } else {
      insert(node, node->children[i], k, i);
      if (node->numKeys == degree) {
        int splitPos = degree / 2;
        Node<degree>* newNode = ALLOC_OBJ(Node<degree>());
        for (int j = splitPos + 1, l = 0; j < node->numKeys; j++, l++) {
          newNode->keys[l] = node->keys[j];
          newNode->children[l] = node->children[j];
          newNode->numKeys++;
        }
        newNode->children[newNode->numKeys] = node->children[node->numKeys];
        node->numKeys = splitPos;
        if (node->flags & NodeFlags::isRoot) {
          node->flags &= ~NodeFlags::isRoot;
          Node<degree>* newRoot = ALLOC_OBJ(Node<degree>(NodeFlags::isRoot));
          newRoot->keys[0] = node->keys[splitPos];
          newRoot->numKeys++;
          newRoot->children[0] = node;
          newRoot->children[1] = newNode;
          root = newRoot;
          height++;
        } else {
          for (int j = parent->numKeys; j > parentChildIndex; j--) {
            parent->keys[j] = parent->keys[j - 1];
            parent->children[j + 1] = parent->children[j];
          }
          parent->keys[parentChildIndex] = node->keys[splitPos];
          parent->numKeys++;
          parent->children[parentChildIndex + 1] = newNode;
        }
      }
    }
  }
  int search(Node<degree>* node, int k) const {
    if (node->flags & NodeFlags::isLeaf) {
      for (int i = 0; i < node->numKeys; i++) {
        if (node->keys[i] == k)
          return k;
      }
      return INT_MIN;
    } else {
      int i = 0;
      while (i < node->numKeys && k >= node->keys[i]) {
        i++;
      }
      return search(node->children[i], k);
    }
  }
  void inorder(std::string& str) const {
    Node<degree>* current = root;
    while ((current->flags & NodeFlags::isLeaf) == 0) {
      current = current->children[0];
    }
    do {
      str += " [";
      int i;
      for (i = 0; i < current->numKeys - 1; i++) {
        str += std::to_string(current->keys[i]);
        str += "|";
      }
      str += std::to_string(current->keys[i]);
      str += "] ";
      current = current->next;
    } while (current);
  }
  void clear(Node<degree>* node) {
    if (node->flags & NodeFlags::isLeaf) {
      node->killSelf();
      return;
    } else {
      for (int i = 0; i <= node->numKeys; i++) {
        clear(node->children[i]);
      }
      node->killSelf();
    }
  }
};

#endif