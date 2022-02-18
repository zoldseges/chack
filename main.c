#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "utils.h"

#define RAM_SZ 32768
#define ROM_SZ 32768

extern int errno;

typedef struct parsed_op {
  char cmd[32];
  char arg1[64];
  char arg2[32];
} parsed_op;

typedef struct class {
  char fpath[256];
  char cname[64];
  parsed_op prog[ROM_SZ];
  int prog_lines;
} class;

// return 0 on success
int lex(class *class) {
  FILE *fp = fopen(class->fpath, "r");
  char *line = NULL;
  size_t len = 0;
  int prog_line = 0;
  char *token = NULL;
  parsed_op *curr = NULL;
  parsed_op *next = class->prog;
  
  while(getline(&line, &len, fp) != -1){
    token = strtok(line, " \n");
    // skip comments and empty lines
    if (token == NULL || token[0] == '/') continue;

    curr = next;
    next++;
    prog_line++;
    strcpy(curr->cmd, token);
    curr->arg1[0] = '\0';
    curr->arg2[0] = '\0';
    token = strtok(NULL, " \n");
    if (token == NULL) continue;
    strcpy(curr->arg1, token);
    token = strtok(NULL, " \n");
    if (token == NULL) continue;
    strcpy(curr->arg2, token);
  }
  class->prog_lines = prog_line;
  fclose(fp);
  return 0;
}

// return class_count
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

/*
  building VM
*/

struct ref {
  char func[64];
  char arg[64];
  uint16_t addr;
};

typedef struct ref_tbl {
  struct ref tbl[ROM_SZ/2];
  int tbl_sz;
} ref_tbl;

void build_ref_tbl(ref_tbl *tbl, class *classes, int class_count) {
  char func[64] = {0};
  int addr = 0;
  tbl->tbl_sz = 0;
  int is_label = 0;
  int is_function = 0;
  struct ref *curr_ref = tbl->tbl;
  for(int i = 0; i < class_count; i++){
    for(int j = 0; j < classes[i].prog_lines; j++){
      is_label = !strcmp(classes[i].prog[j].cmd, "label");
      is_function = !strcmp(classes[i].prog[j].cmd, "function");
      if (is_label || is_function) {
	if (is_function) {
	  strcpy(func, classes[i].prog[j].arg1);
	}
	strcpy(curr_ref->func, func);
	strcpy(curr_ref->arg, classes[i].prog[j].arg1);
	curr_ref->addr = addr;
	tbl->tbl_sz++;
	curr_ref = &tbl->tbl[tbl->tbl_sz];
      }
      addr++;
    }
  }
}

/*
  virtual machine
*/

enum CMD {
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF,
  C_FUNCTION,
  C_CALL,
  C_RETURN,
};

enum A_ARITHMETIC {
  A_ADD,
  A_SUB,
  A_NEG,
  A_EQ,
  A_GT,
  A_LT,
  A_AND,
  A_OR,
  A_NOT
};

enum A_SEGMENT {
  A_ARG,
  A_LCL,
  A_STATIC,
  A_CONST,
  A_THIS,
  A_THAT,
  A_POINTER,
  A_TEMP
};
  
typedef struct encoded_op {
  enum CMD cmd;
  uint16_t arg1;
  uint16_t arg2;
} op;

typedef struct VM {
  int16_t ram[RAM_SZ];
  op prog[ROM_SZ];
  int pc;
} VM;

void unreachable_branch_error(parsed_op *p_op, char *filename, int line){
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "%s:%d\n", filename, line);
  fprintf(stderr, "Unreachable branch. cmd: %s arg1: %s arg2: %s\n",
	  p_op->cmd, p_op->arg1, p_op->arg2);
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

void encode_arithmetic(parsed_op *p_op, op *e_op){
  if (strcmp(p_op->cmd, "add") == 0) {
    e_op->arg1 = A_ADD;
  } else if (strcmp(p_op->cmd, "sub") == 0) {
    e_op->arg1 = A_SUB;
  } else if (strcmp(p_op->cmd, "neg") == 0) {
    e_op->arg1 = A_NEG;
  } else if (strcmp(p_op->cmd, "eq") == 0) {
    e_op->arg1 = A_EQ;
  } else if (strcmp(p_op->cmd, "gt") == 0) {
    e_op->arg1 = A_GT;
  } else if (strcmp(p_op->cmd, "lt") == 0) {
    e_op->arg1 = A_LT;
  } else if (strcmp(p_op->cmd, "and") == 0) {
    e_op->arg1 = A_AND;
  } else if (strcmp(p_op->cmd, "or") == 0) {
    e_op->arg1 = A_OR;
  } else if (strcmp(p_op->cmd, "not") == 0) {
    e_op->arg1 = A_NOT;
  } else {
    unreachable_branch_error(p_op, __FILE__, __LINE__);
  }
}

void encode_pushpop(parsed_op *p_op, op *e_op){
  if (strcmp(p_op->arg1, "argument") == 0) {
    e_op->arg1 = A_ARG;
  } else if (strcmp(p_op->arg1, "local") == 0) {
    e_op->arg1 = A_LCL;
  } else if (strcmp(p_op->arg1, "static") == 0) {
    e_op->arg1 = A_STATIC;
  } else if (strcmp(p_op->arg1, "constant") == 0) {
    e_op->arg1 = A_CONST;
  } else if (strcmp(p_op->arg1, "this") == 0) {
    e_op->arg1 = A_THIS;
  } else if (strcmp(p_op->arg1, "that") == 0) {
    e_op->arg1 = A_THAT;
  } else if (strcmp(p_op->arg1, "pointer") == 0) {
    e_op->arg1 = A_POINTER;
  } else if (strcmp(p_op->arg1, "temp") == 0) {
    e_op->arg1 = A_TEMP;
  } else {
    unreachable_branch_error(p_op, __FILE__, __LINE__);
  }
}

// return 1 on error
void encode_ref(parsed_op *p_op, op *e_op, ref_tbl *ref_tbl, char *curr_func){
  for(int i = 0; i < ref_tbl->tbl_sz; i++){
    if( strcmp(ref_tbl->tbl[i].func, curr_func) == 0 &&
	strcmp(ref_tbl->tbl[i].arg, p_op->arg1) == 0){
      e_op->arg1 = ref_tbl->tbl[i].addr;
      return;
    }
  }
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
  fprintf(stderr, "Reference can't be resolved: %s %s %s\n", p_op->cmd, p_op->arg1, p_op->arg2);
  exit(1);
}

void encode_cmd(parsed_op *p_op, op *e_op, ref_tbl *ref_tbl, char *curr_func){
  int arg2 = 0;
  if (strcmp(p_op->cmd, "add") == 0 ||
      strcmp(p_op->cmd, "sub") == 0 ||
      strcmp(p_op->cmd, "neg") == 0 ||
      strcmp(p_op->cmd, "eq") == 0 ||
      strcmp(p_op->cmd, "gt") == 0 ||
      strcmp(p_op->cmd, "lt") == 0 ||
      strcmp(p_op->cmd, "and") == 0 ||
      strcmp(p_op->cmd, "or") == 0 ||
      strcmp(p_op->cmd, "not") == 0) {
    e_op->cmd = C_ARITHMETIC;
    encode_arithmetic(p_op, e_op);
  } else if (strcmp(p_op->cmd, "push") == 0) {
    arg2 = atoi(p_op->arg2);
    atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    e_op->cmd = C_PUSH;
    e_op->arg2 = arg2;
    encode_pushpop(p_op, e_op);
  } else if (strcmp(p_op->cmd, "pop") == 0) {
    arg2 = atoi(p_op->arg2);
    atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    e_op->cmd = C_POP;
    e_op->arg2 = arg2;
    encode_pushpop(p_op, e_op);
  } else if (strcmp(p_op->cmd, "label") == 0) {
    e_op->cmd = C_LABEL;
    encode_ref(p_op, e_op, ref_tbl, curr_func);
  } else if (strcmp(p_op->cmd, "goto") == 0) {
    e_op->cmd = C_GOTO;
    encode_ref(p_op, e_op, ref_tbl, curr_func);
  } else if (strcmp(p_op->cmd, "if-goto") == 0) {
    e_op->cmd = C_IF;
    encode_ref(p_op, e_op, ref_tbl, curr_func);
  } else if (strcmp(p_op->cmd, "function") == 0) {
    arg2 = atoi(p_op->arg2);
    atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    e_op->cmd = C_FUNCTION;
    e_op->arg2 = arg2;
    strcpy(curr_func, p_op->arg1);
    encode_ref(p_op, e_op, ref_tbl, curr_func);
  } else if (strcmp(p_op->cmd, "call") == 0) {
    arg2 = atoi(p_op->arg2);
    atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    e_op->cmd = C_CALL;
    e_op->arg2 = arg2;
    encode_ref(p_op, e_op, ref_tbl, p_op->arg1);
  } else if (strcmp(p_op->cmd, "return") == 0) {
    e_op->cmd = C_RETURN;
  } else {
    unreachable_branch_error(p_op, __FILE__, __LINE__);
  }
}

void build_vm(VM *vm, ref_tbl *ref_tbl, class *classes, int class_count) {
  vm->pc = 0;
  char curr_func[64] = {0};
  for(int i = 0; i < class_count; i++){
    for(int j = 0; j < classes[i].prog_lines; j++){
      encode_cmd(&classes[i].prog[j], &vm->prog[vm->pc], ref_tbl, curr_func);
      vm->pc++;
    }
  }
}

void print_parsed_op(parsed_op op){
  printf("%s", op.cmd);
  if (*op.arg1) printf(" %s", op.arg1);
  if (*op.arg2) printf(" %s", op.arg2);
  printf("\n");
}

void print_encoded_op(op *op, ref_tbl *ref_tbl){
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
      printf("argument ");
      break;
    case A_LCL:
      printf("local ");
      break;
    case A_STATIC:
      printf("static ");
      break;
    case A_CONST:
      printf("constant ");
      break;
    case A_THIS:
      printf("this ");
      break;
    case A_THAT:
      printf("that ");
      break;
    case A_POINTER:
      printf("pointer ");
      break;
    case A_TEMP:
      printf("temp ");
      break;
    }
    printf("%d\n", op->arg2);
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
	printf("%s\n", ref_tbl->tbl[i].arg);
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
	printf("%s ", ref_tbl->tbl[i].arg);
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

int main(int argc, char *argv[]){
  class *parsed_classes = NULL;
  int class_count = 0;
  int errnum; 
  char *input = NULL;
  ref_tbl ref_tbl;
  VM vm = {0};
  // no input given
  if (argc < 2) {
    exit(1);
  } else {
    input = argv[1];
  }

  parsed_classes = malloc(sizeof(class) * 64);
  class_count = get_class_paths(parsed_classes, input);
  for(int i = 0; i < class_count; i++){
    lex(&parsed_classes[i]);
  }

  /* int ln = 0; */
  /* for(int i = 0; i < class_count; i++){ */
  /*   printf("----- %-16s -----\n", parsed_classes[i].cname); */
  /*   for(int j = 0; j < parsed_classes[i].prog_lines; j++){ */
  /*     printf("%-12d", ln++); */
  /*     print_parsed_op(parsed_classes[i].prog[j]); */
  /*   } */
  /* } */

  /* printf("-----------------------------\n"); */

  build_ref_tbl(&ref_tbl, parsed_classes, class_count);

  /* for(int i = 0; i < ref_tbl.tbl_sz; i++){ */
  /*   struct ref *r = &ref_tbl.tbl[i]; */
  /*   printf("%-8d %-32s %-16s\n", r->addr, r->func, r->arg); */
  /* } */

  build_vm(&vm, &ref_tbl, parsed_classes, class_count);
  for(int i = 0; i < vm.pc; i++) {
    print_encoded_op(&vm.prog[i], &ref_tbl);
    /* printf("%d %d %d\n", vm.prog[i].cmd, vm.prog[i].arg1, vm.prog[i].arg2); */
  }
  free(parsed_classes);
  return 0;
}
