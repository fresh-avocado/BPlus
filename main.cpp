#include <iostream>

#include "BPlus.hpp"

int main() {
  BPlusTree<5> bplus;

  int records[101];
  for (int i = 1; i <= 100; i++) {
    records[i] = i;
    bplus.insert(i, &records[i]);
    assert(bplus.search(i) == &records[i]);
  }

  printf("inorder string: %s\n", bplus.inorderString().c_str());
  printf("height: %d\n", bplus.getHeight());
  printf("root: %s\n", bplus.getRoot().c_str());

  for (auto i : bplus.range(LowerBound::GT, 50, UpperBound::LT, 52)) {
    printf("%d,", *(int*)i);
  }
  printf("\n");

  return 0;
}