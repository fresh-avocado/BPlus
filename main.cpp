#include <iostream>

#include "BPlus.cpp"

int main() {
  // setbuf(stdout, NULL);
  BPlusTree<3> bplus;

  int records[101];
  for (int i = 1; i <= 100; i++) {
    records[i] = i;
    // bplus.insert(i, &records[i]);
    // assert(bplus.search(i) == &records[i]);
  }

  bplus.insert(1, &records[1]);
  bplus.insert(20, &records[20]);
  bplus.insert(49, &records[49]);
  bplus.insert(21, &records[21]);
  bplus.insert(11, &records[11]);
  bplus.insert(9, &records[9]);

  printf("inorder string: %s\n", bplus.inorderString().c_str());
  printf("height: %d\n", bplus.getHeight());
  printf("root: %s\n", bplus.getRoot().c_str());

  for (auto i : bplus.range(SearchCriteria::GT, 9)) {
    printf("%d,", *(int*)i);
  }
  printf("\n");

  return 0;
}