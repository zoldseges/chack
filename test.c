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
  assert(vm.ram[16 ] == 888);
  assert(vm.ram[17 ] == 333);
  assert(vm.ram[18 ] == 111);
  printf("%s passed\n", input);
}

void test_pointertest(){
  char *input = "test_input/memoryaccess/PointerTest.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0] = 256;
  run(&vm);
  assert(vm.ram[256 ] == 6084);
  assert(vm.ram[3   ] == 3030);
  assert(vm.ram[4   ] == 3040);
  assert(vm.ram[3032] == 32  );
  assert(vm.ram[3046] == 46  );
  printf("%s passed\n", input);
}


void test_basicloop(){
  char *input = "test_input/programflow/BasicLoop.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0  ] = 256;
  vm.ram[1  ] = 300;
  vm.ram[2  ] = 400;
  vm.ram[400] = 3  ;
  run(&vm);
  assert(vm.ram[0  ] == 257);
  assert(vm.ram[256] == 6  );
  printf("%s passed\n", input);
}

void test_fibonacciseries(){
  char *input = "test_input/programflow/FibonacciSeries.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0  ] = 256 ;
  vm.ram[1  ] = 300 ;
  vm.ram[2  ] = 400 ;
  vm.ram[400] = 6   ;
  vm.ram[401] = 3000;
  run(&vm);
  assert(vm.ram[3000] == 0);
  assert(vm.ram[3001] == 1);
  assert(vm.ram[3002] == 1);
  assert(vm.ram[3003] == 2);
  assert(vm.ram[3004] == 3);
  assert(vm.ram[3005] == 5);
  printf("%s passed\n", input);
}

void test_simplefunction(){
  char *input = "test_input/functioncalls/SimpleFunction.vm";
  VM vm = {0};
  build_vm_from_fpath(&vm, input);
  vm.ram[0  ] = 317 ;
  vm.ram[1  ] = 317 ;
  vm.ram[2  ] = 310 ;
  vm.ram[3  ] = 3000;
  vm.ram[4  ] = 4000;
  vm.ram[310] = 1234;
  vm.ram[311] = 37  ;
  /* vm.ram[312] = 1000; */
  vm.ram[312] = 9   ;
  vm.ram[313] = 305 ;
  vm.ram[314] = 300 ;
  vm.ram[315] = 3010;
  vm.ram[316] = 4010;
  for(int i = 0; i < 10; i++){
    step(&vm);
  }
  assert(vm.ram[0  ] == 311 );
  assert(vm.ram[1  ] == 305 );
  assert(vm.ram[2  ] == 300 );
  assert(vm.ram[3  ] == 3010);
  assert(vm.ram[4  ] == 4010);
  assert(vm.ram[310] == 1196);
  printf("%s passed\n", input);
}
int main() {
  test_assign_static();
  test_stacktest();
  test_basictest();
  test_statictest();
  test_pointertest();
  test_basicloop();
  test_fibonacciseries();
  test_simplefunction();
  return 0;
}
