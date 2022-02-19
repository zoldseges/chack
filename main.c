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

  for(int i = 0; i < vm.prog_lines; i++) {
    step(&vm);
  }
  
  printf("RAM[0]=%d\n", vm.ram[0]);
  printf("RAM[256]=%d\n", vm.ram[256]);
  printf("RAM[257]=%d\n", vm.ram[257]);
  printf("RAM[258]=%d\n", vm.ram[258]);
  printf("RAM[259]=%d\n", vm.ram[259]);
  printf("RAM[260]=%d\n", vm.ram[260]);
  printf("RAM[261]=%d\n", vm.ram[261]);
  printf("RAM[262]=%d\n", vm.ram[262]);
  printf("RAM[263]=%d\n", vm.ram[263]);
  printf("RAM[264]=%d\n", vm.ram[264]);
  printf("RAM[265]=%d\n", vm.ram[265]);
  free(classes);
  return 0;
}
