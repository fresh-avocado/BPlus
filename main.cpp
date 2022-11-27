#include <iostream>

#include "BPlus.cpp"

auto measureTime(const char* benchmarkName,
                 const std::function<void(void)>& compute) -> void {
  // NOLINTNEXTLINE
  auto start = std::chrono::system_clock::now();
  compute();
  // NOLINTNEXTLINE
  auto end = std::chrono::system_clock::now();

  printf("%s took: %lf ms\n", benchmarkName,
         std::chrono::duration_cast<std::chrono::duration<double>>(end - start)
                 .count() *
             1000.0);
}

int main() {
  // setbuf(stdout, NULL);
  BPlusTree<3> bplus;

  int records[101];
  measureTime("insertion time", [&] {
    for (int i = 1; i <= 1'000'000; i++) {
      // records[i] = i;
      bplus.insert(i, records);
      // assert(bplus.search(i) == &records[i]);
    }
  });

  // bplus.insert(1, &records[1]);
  // bplus.insert(20, &records[20]);
  // bplus.insert(49, &records[49]);
  // bplus.insert(21, &records[21]);
  // bplus.insert(11, &records[11]);
  // bplus.insert(9, &records[9]);

  // printf("inorder string: %s\n", bplus.inorderString().c_str());
  printf("height: %d\n", bplus.getHeight());
  // printf("root: %s\n", bplus.getRoot().c_str());

  // for (auto i : bplus.range(SearchCriteria::GT, 9)) {
  //   printf("%d,", *(int*)i);
  // }
  // printf("\n");

  return 0;
}