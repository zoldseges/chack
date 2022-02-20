#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "types.h"
#include "lex.h"
#include "vm.h"

/*
  returns 0 on error
  1 if file
  2 if dir
*/
int is_file_or_dir(char *path) {
  struct stat statbuf;
  int res = 0;
  if (stat(path, &statbuf) != 0) return 0;
  res = S_ISREG(statbuf.st_mode);
  if(res) return 1;
  res = S_ISDIR(statbuf.st_mode);
  if(res) return 2;
  return res;
}

/*
  fills classes fname and cname 
  returns class_count
*/
int get_class_paths(class *classes, char *input) {
  int count = 0;
  int file_or_dir = is_file_or_dir(input);
  char path[256];
  if (file_or_dir == 1) {
    char *cname = {0};
    count = 1;
    strcpy(classes[0].fpath, input);
    for(int i = 0; input[i] != '\0'; i++){
      if(i > 0 && input[i-1] == '/'){
	cname = &input[i];
      }
    }
    strcpy(classes[0].cname, cname);
    strtok(classes[0].cname, ".");
  } else if (file_or_dir == 2) {
    //set last char to "/"
    strcpy(path, input);
    DIR *d;
    struct dirent *dir;
    char ext[4] = {0};
    int string_i = 0;
    //set last char to "/"
    while(path[string_i] != '\0') string_i++;
    if (path[string_i -1] != '/') {
      path[string_i++] = '/';
      path[string_i] = '\0';
    }
    string_i = 0;
    d = opendir(input);
    if(d == NULL) {
      fprintf(stderr, "Can't open dir %s\n", input);
      perror("Error:");
      exit(1);
    }
    while((dir = readdir(d)) != NULL){
      string_i = 0;
      while(dir->d_name[string_i] != '\0') string_i++;
      // direntry shorter than 3 chars
      if (string_i < 3) continue;
      for(int j = 0; j < 4; j++) ext[j] = dir->d_name[string_i - 3 + j];
      // filename doesn't end with .vm
      if (strcmp(ext, ".vm") != 0) continue;
      strcpy(classes[count].fpath, path);
      strcat(classes[count].fpath, dir->d_name);
      strcpy(classes[count].cname, dir->d_name);
      strtok(classes[count].cname, ".");
      count++;
    }
  } else {
    fprintf(stderr, "Can't handle the given path: %s\n", input);
    exit(1);
  }
  return count;
}

void unreachable_branch_error(char *msg, char *filename, int line){
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "%s:%d\n", filename, line);
  fprintf(stderr, "Unreachable branch: %s\n", msg);
  exit(1);
}

void atoi_error(char *snum, int n, char *filename, int line){
  if(n == 0 && snum[0] != '0'){
    fprintf(stderr, "ERROR\n");
    fprintf(stderr, "%s:%d\n", filename, line);
    fprintf(stderr, "ATOI error: %s\n", snum);
    exit(1);
  }
}

void __print_parsed_op(parsed_op op){
  printf("%s", op.cmd);
  if (*op.arg1) printf(" %s", op.arg1);
  if (*op.arg2) printf(" %s", op.arg2);
  printf("\n");
}

void print_parsed_prog(parsed_classes *classes){
  for(int i = 0; i < classes->class_count; i++){
    for(int j = 0; j < classes->classes[i].prog_lines; j++){
      __print_parsed_op(classes->classes[i].prog[j]);
    }
  }
}

void __print_encoded_op(op *op, ref_tbl *ref_tbl){
  int flag = 0;
  switch(op->cmd){
  case C_ARITHMETIC:
    switch(op->arg1){
    case A_ADD:
      printf("add\n");
      break;
    case A_SUB:
      printf("sub\n");
      break;
    case A_NEG:
      printf("neg\n");
      break;
    case A_EQ:
      printf("eq\n");
      break;
    case A_GT:
      printf("gt\n");
      break;
    case A_LT:
      printf("lt\n");
      break;
    case A_AND:
      printf("and\n");
      break;
    case A_OR:
      printf("or\n");
      break;
    case A_NOT:
      printf("not\n");
      break;
    }
    break;
  case C_PUSH:
    printf("push ");
    flag = 1;
  case C_POP:
    if (!flag) printf("pop ");
    switch(op->arg1){
    case A_ARG:
      printf("argument %d\n",	op->arg2);
      break;
    case A_LCL:
      printf("local %d\n",	op->arg2);
      break;
    case A_STATIC:
      int arg2 = 0;
      printf("static %d\n",     arg2);
      break;
    case A_CONST:
      printf("constant %d\n",	op->arg2);
      break;
    case A_THIS:
      printf("this %d\n",	op->arg2);
      break;
    case A_THAT:
      printf("that %d\n",	op->arg2);
      break;
    case A_POINTER:
      printf("pointer %d\n",	op->arg2);
      break;
    case A_TEMP:
      printf("temp %d\n",	op->arg2);
      break;
    }
    break;
  case C_LABEL:
    printf("label ");
    flag = 1;
  case C_GOTO:
    if(!flag) {
      printf("goto ");
      flag = 1;
    }
  case C_IF:
    if(!flag) printf("if-goto ");
    for(int i = 0; i < ref_tbl->tbl_sz; i++){
      if(ref_tbl->tbl[i].addr == op->arg1) {
	printf("%s\n", ref_tbl->tbl[i].arg1);
	break;
      }
    }
    break;
  case C_FUNCTION:
    printf("function ");
    flag = 1;
  case C_CALL:
    if(!flag) printf("call ");
    for(int i = 0; i < ref_tbl->tbl_sz; i++){
      if(ref_tbl->tbl[i].addr == op->arg1) {
	printf("%s ", ref_tbl->tbl[i].arg1);
	break;
      }
    }
    printf("%d\n", op->arg2);
    break;
  case C_RETURN:
    printf("return\n");
    break;
  }
}

void print_vm_prog(VM *vm, parsed_classes *classes){
  for(int i = 0; i < vm->prog_lines; i++){
    __print_encoded_op(&vm->prog[i], &classes->ref_tbl);
  }
}

// returns 0 on success
int build_vm_from_fpath(VM *vm, char *input){
  parsed_classes *classes = NULL;
  classes = malloc(sizeof(parsed_classes));
  if(classes == NULL){
    fprintf(stderr, "ERROR\n");
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Error during malloc\n");
    exit(1);
  }

  if(parse_classes(classes, input)) {
    fprintf(stderr, "ERROR\n");
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Error during parsing classes: %s\n", input);
    exit(1);
  }

  if(build_vm_from_classes(vm, classes)) {
    fprintf(stderr, "ERROR\n");
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Error during parsing classes: %s\n", input);
    exit(1);
  }
  free(classes);
  return 0;
}
