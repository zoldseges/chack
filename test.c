#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils.h"
#include "types.h"
#include "lex.h"
#include "vm.h"

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

void test_stacktest(){
  char *input = "test_input/stackarithmetic/StackTest.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0] = 256;
  run(&vm);
  assert(vm.ram[0  ] == 266);
  assert(vm.ram[256] == -1 );
  assert(vm.ram[257] == 0  );
  assert(vm.ram[258] == 0  );
  assert(vm.ram[259] == 0  );
  assert(vm.ram[260] == -1 );
  assert(vm.ram[261] == 0  );
  assert(vm.ram[262] == -1 );
  assert(vm.ram[263] == 0  );
  assert(vm.ram[264] == 0  );
  assert(vm.ram[265] == -91);
  printf("%s passed\n", input);
}

void test_basictest(){
  char *input = "test_input/memoryaccess/BasicTest.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0] = 256;
  vm.ram[1] = 300;
  vm.ram[2] = 400;
  vm.ram[3] = 3000;
  vm.ram[4] = 3010;
  run(&vm);
  assert(vm.ram[256 ]	== 472);
  assert(vm.ram[300 ]	== 10 );
  assert(vm.ram[401 ]	== 21 );
  assert(vm.ram[402 ]	== 22 );
  assert(vm.ram[3006]	== 36 );
  assert(vm.ram[3012]	== 42 );
  assert(vm.ram[3015]	== 45 );
  assert(vm.ram[11  ]	== 510);
  printf("%s passed\n", input);
}

void test_statictest(){
  char *input = "test_input/memoryaccess/StaticTest.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0] = 256;
  run(&vm);
  assert(vm.ram[256] == 1110);
  for(int i = 16; i < 25; i++){
    printf("%d: %d\n", i, vm.ram[i]);
  }
  printf("%s passed\n", input);
}

int main() {
  test_assign_static();
  test_stacktest();
  test_basictest();
  test_statictest();
  return 0;
}
