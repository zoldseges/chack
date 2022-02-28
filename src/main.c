#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include "types.h"
#include "utils.h"
#include "config.h"
#include "lex.h"
#include "vm.h"
#include "vmio.h"

int main(int argc, char *argv[]){
  char *input = NULL;
  VM vm = {0};
  
  /* no input given */
  if (argc < 2) {
    exit(1);
  } else {
    input = argv[1];
  }

  build_vm_from_fpath(&vm, input);
  vm.run = 1;
  connect_io(&vm);
  while(vm.run){
    step(&vm);
  }

  return 0;
}
