#include <iostream>

#include "BPlus.hpp"

int main() {
  BPlusTree<5> bplus;

  for (int i = 1; i <= 40; i++) {
    bplus.insert(i);
    assert(bplus.search(i) == i);
  }

  printf("inorder string: %s\n", bplus.inorderString().c_str());
  printf("height: %d\n", bplus.getHeight());
  printf("root: %s\n", bplus.getRoot().c_str());

  return 0;
}