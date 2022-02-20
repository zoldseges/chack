#include <stdio.h>
#include <string.h>
#include <assert.h>

int assign_static(char *, char *);

void test_assign_static(){
  // test int assign_static(char *, char*);
  char *input[] = {
    "A1", "2", // 16
    "A1", "2", // 16
    "B1", "0", // 17
    "A1", "1", // 18
    "C1", "3", // 19
    "B1", "0", // 17
    "A2", "2", // 20
    "A1", "1", // 18
    "A1", "2", // 16
    "END",
  };
  
  assert(16 == assign_static(input[ 0], input[ 1]));
  assert(16 == assign_static(input[ 2], input[ 3]));
  assert(17 == assign_static(input[ 4], input[ 5]));
  assert(18 == assign_static(input[ 6], input[ 7]));
  assert(19 == assign_static(input[ 8], input[ 9]));
  assert(17 == assign_static(input[10], input[11]));
  assert(20 == assign_static(input[12], input[13]));
  assert(18 == assign_static(input[14], input[15]));
  assert(16 == assign_static(input[16], input[17]));
  printf("%s passed\n", __func__);
}

int main() {
  test_assign_static();
  return 0;
}
