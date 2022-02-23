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

int main(int argc, char *argv[]){
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

  if(build_vm_from_classes(&vm, classes)) {
    fprintf(stderr, "Error during parsing classes: %s\n", input);
    exit(1);
  }
  /* print_parsed_prog(classes); */
  print_vm_prog(&vm, classes);

  /* for(int i = 0; i < classes->ref_tbl.tbl_sz; i++) { */
  /*   struct ref *ref = &classes->ref_tbl.tbl[i]; */
  /*   printf("%-12s %-32s %-24s %-12s, %d\n", ref->class, ref->func, ref->arg1, ref->arg2, ref->addr); */
  /* } */

  /*test*/
  vm.ram[0] = 256;

  /* for(int i = 0; i < vm.prog_lines; i++) { */
  /*   step(&vm); */
  /* } */
  
  free(classes);
  return 0;
}
