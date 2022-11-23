#ifndef BPLUS_HPP_
#define BPLUS_HPP_

#include <stdlib.h>
#include <string>
#include <vector>

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

enum class SearchCriteria { GT, GTE, LT, LTE };
enum class LowerBound { GT, GTE };
enum class UpperBound { LT, LTE };

struct NodeFlags {
  enum : std::uint8_t {
    isRoot = 1 << 1,
    isLeaf = 1 << 2,
  };
};
struct RangeSearchFlags {
  enum : std::uint8_t {
    GT = 1 << 1,
    GTE = 1 << 2,
    LT = 1 << 3,
    LTE = 1 << 4,
  };
};

template <int degree>
class BPlusTree;

template <int degree>
class Node {
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
  // TODO: inorderRecords (ASC, DESC)
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
  // TODO: ASC o DESC
  std::vector<void*> range(SearchCriteria criteria, int bound) {
    return rangeSearchHelper(root, bound, criteria);
  }
  std::vector<void*> range(LowerBound lowerSearch,
                           int lowerBound,
                           UpperBound upperSearch,
                           int upperBound) {
    return rangeSearchHelper(root, lowerSearch, lowerBound, upperSearch,
                             upperBound);
  }
  void* search(int k) const { return search(root, k); }
  void insert(int k, void* record) { insert(nullptr, root, k, record, -1); }
  ~BPlusTree() noexcept { clear(); }

 private:
  std::vector<void*> rangeSearchHelper(Node<degree>* node,
                                       int bound,
                                       SearchCriteria criteria) {
    if (node->flags & NodeFlags::isLeaf) {
      std::vector<void*> res;
      int j = 0;
      if (criteria == SearchCriteria::GT) {
        while (j < node->numKeys && node->keys[j] <= bound) {
          j++;
        }
        do {
          while (j < node->numKeys) {
            res.push_back(node->children[j]);
            j++;
          }
          j = 0;
          node = node->next;
        } while (node);
      } else if (criteria == SearchCriteria::GTE) {
        while (j < node->numKeys && node->keys[j] < bound) {
          j++;
        }
        do {
          while (j < node->numKeys) {
            res.push_back(node->children[j]);
            j++;
          }
          j = 0;
          node = node->next;
        } while (node);
      } else if (criteria == SearchCriteria::LT) {
        j = node->numKeys - 1;
        while (j >= 0 && node->keys[j] >= bound) {
          j--;
        }
        do {
          while (j >= 0) {
            res.push_back(node->children[j]);
            j--;
          }
          node = node->prev;
          j = node ? node->numKeys - 1 : 0;
        } while (node);
      } else if (criteria == SearchCriteria::LTE) {
        j = node->numKeys - 1;
        while (j >= 0 && node->keys[j] > bound) {
          j--;
        }
        do {
          while (j >= 0) {
            res.push_back(node->children[j]);
            j--;
          }
          node = node->prev;
          j = node ? node->numKeys - 1 : 0;
        } while (node);
      }
      return res;
    } else {
      int i = 0;
      while (i < node->numKeys && bound >= node->keys[i]) {
        i++;
      }
      return rangeSearchHelper(node->children[i], bound, criteria);
    }
  }
  std::vector<void*> rangeSearchHelper(Node<degree>* node,
                                       LowerBound lowerSearch,
                                       int lowerBound,
                                       UpperBound upperSearch,
                                       int upperBound) {
    if (node->flags & NodeFlags::isLeaf) {
      int j = 0;
      if (lowerSearch == LowerBound::GT) {
        while (j < node->numKeys && node->keys[j] <= lowerBound) {
          j++;
        }
      } else if (lowerSearch == LowerBound::GTE) {
        while (j < node->numKeys && node->keys[j] < lowerBound) {
          j++;
        }
      }
      std::vector<void*> res;
      if (upperSearch == UpperBound::LT) {
        do {
          while (j < node->numKeys) {
            if (node->keys[j] >= upperBound) {
              goto DONE;
            }
            res.push_back(node->children[j]);
            j++;
          }
          j = 0;
          node = node->next;
        } while (node);
      } else if (upperSearch == UpperBound::LTE) {
        do {
          while (j < node->numKeys) {
            if (node->keys[j] > upperBound) {
              goto DONE;
            }
            res.push_back(node->children[j]);
            j++;
          }
          j = 0;
          node = node->next;
        } while (node);
      }
    DONE:
      return res;
    } else {
      int i = 0;
      while (i < node->numKeys && lowerBound >= node->keys[i]) {
        i++;
      }
      return rangeSearchHelper(node->children[i], lowerSearch, lowerBound,
                               upperSearch, upperBound);
    }
  }

  void insert(Node<degree>* parent,
              Node<degree>* node,
              int k,
              void* record,
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
      node->children[i] = (Node<degree>*)record;
      if (++node->numKeys == degree) {
        int splitPos = degree / 2;
        Node<degree>* newNode = ALLOC_OBJ(Node<degree>(NodeFlags::isLeaf));
        int pos;
        for (int j = splitPos; j < node->numKeys; j++) {
          pos = j - splitPos;
          newNode->keys[pos] = node->keys[j];
          newNode->children[pos] = node->children[j];
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
      insert(node, node->children[i], k, record, i);
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
  void* search(Node<degree>* node, int k) const {
    if (node->flags & NodeFlags::isLeaf) {
      for (int i = 0; i < node->numKeys; i++) {
        if (node->keys[i] == k)
          return (void*)node->children[i];
      }
      return nullptr;
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