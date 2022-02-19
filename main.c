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

  /* if(build_vm(&vm, classes)) { */
  /*   fprintf(stderr, "Error during parsing classes: %s\n", input); */
  /*   exit(1); */
  /* } */
  /* print_vm_prog(&vm, classes); */
  free(classes);
  return 0;
}
