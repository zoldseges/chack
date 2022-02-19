#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "types.h"
#include "utils.h"
#include "config.h"
#include "lex.h"
#include "vm.h"

extern int errno;

int main(int argc, char *argv[]){
  int errnum; 
  char *input = NULL;
  parsed_classes *classes = NULL;
  classes = malloc(sizeof(parsed_classes));
  VM vm = {0};

  // no input given
  if (argc < 2) {
    exit(1);
  } else {
    input = argv[1];
  }

  if(parse_classes(classes, input)) {
    fprintf(stderr, "Error during parsing classes: %s\n", input);
    exit(1);
  }

  /* print_parsed_prog(classes); */

  /* printf("%d\n", classes->ref_tbl.tbl_sz); */
  /* for(int i = 0; i < classes->ref_tbl.tbl_sz; i++){ */
  /*   printf("%-8d %-32s %-32s\n", classes->ref_tbl.tbl[i].addr, */
  /* 	   classes->ref_tbl.tbl[i].func, */
  /* 	   classes->ref_tbl.tbl[i].arg); */
  /* } */

  if(build_vm(&vm, classes)) {
    fprintf(stderr, "Error during parsing classes: %s\n", input);
    exit(1);
  }
  /* print_vm_prog(&vm, classes); */

  /*test*/
  vm.ram[0] = 256;
  vm.ram[1] = 300;
  vm.ram[2] = 400;
  vm.ram[3] = 3000;
  vm.ram[4] = 3010;

  for(int i = 0; i < vm.prog_lines; i++) {
    step(&vm);
  }
  
  printf("RAM[0] =%d\n", vm.ram[0]);
  printf("RAM[1] =%d\n", vm.ram[1]);
  printf("RAM[2] =%d\n", vm.ram[2]);
  printf("RAM[3] =%d\n", vm.ram[3]);
  printf("RAM[4] =%d\n", vm.ram[4]);
  printf("-------------------------\n");
  printf("RAM[256] =%d\n", vm.ram[256]);
  printf("RAM[300] =%d\n", vm.ram[300]);
  printf("RAM[401] =%d\n", vm.ram[401]);
  printf("RAM[402] =%d\n", vm.ram[402]);
  printf("RAM[3006]=%d\n", vm.ram[3006]);
  printf("RAM[3012]=%d\n", vm.ram[3012]);
  printf("RAM[3015]=%d\n", vm.ram[3015]);
  printf("RAM[11]  =%d\n", vm.ram[11]);
  free(classes);
  return 0;
}
