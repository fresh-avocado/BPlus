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
  BPlusTree();
  std::string inorderString() const;
  // TODO: inorderRecords (ASC, DESC)
  void clear();
  int getHeight() const;
  std::string getRoot() const;
  // TODO: ASC o DESC
  std::vector<void*> range(SearchCriteria criteria, int bound);
  // TODO: ASC o DESC
  std::vector<void*> range(LowerBound lowerSearch,
                           int lowerBound,
                           UpperBound upperSearch,
                           int upperBound);
  void* search(int k) const;
  void insert(int k, void* record);
  ~BPlusTree() noexcept;

 private:
  std::vector<void*> rangeSearchHelper(Node<degree>* node,
                                       int bound,
                                       SearchCriteria criteria);
  std::vector<void*> rangeSearchHelper(Node<degree>* node,
                                       LowerBound lowerSearch,
                                       int lowerBound,
                                       UpperBound upperSearch,
                                       int upperBound);
  void insert(Node<degree>* parent,
              Node<degree>* node,
              int k,
              void* record,
              int parentChildIndex);
  void* search(Node<degree>* node, int k) const;
  void inorder(std::string& str) const;
  void clear(Node<degree>* node);
};

#endif