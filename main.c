#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define RAM_SZ 32768
#define ROM_SZ 32768
#define SP        256
#define SP_BASE     0
#define LCL_BASE    1
#define ARG_BASE    1
#define THIS_BASE   1
#define THAT_BASE   1
#define TEMP_BASE   1
#define R13        13
#define R14        14
#define R15        15

extern int errno;

enum C_TYPE {
  C_ARITHMETIC,
  C_PUSH,
  C_POP,
  C_LABEL,
  C_GOTO,
  C_IF,
  C_FUNCTION,
  C_RETURN,
  C_CALL,
};

struct Operation {
  enum C_TYPE type;
  char arg1[32];
  int16_t arg2;
};

void usage(void) {
  printf("Usage: hackvm <file>\n");
}


void print_op(struct Operation op){
  char cmd[32] = {0};
  
  switch (op.type) {
    case C_ARITHMETIC:
      printf("  %s\n", op.arg1);
      break;
    case C_PUSH:
      printf("  push %s %d\n", op.arg1, op.arg2);
      break;
    case C_POP:
      printf("  pop %s %d\n", op.arg1, op.arg2);
      break;
    case C_LABEL:
      printf("label %s\n", op.arg1);
      break;
    case C_GOTO:
      printf("  goto %s\n", op.arg1);
      break;
    case C_IF:
      printf("  if-goto %s\n", op.arg1);
      break;
    case C_FUNCTION:
      printf("function %s %d\n", op.arg1, op.arg2);
      break;
    case C_RETURN:
      printf("  return\n");
      break;
    case C_CALL:
      printf("  call %s %d\n", op.arg1, op.arg2);
      break;
  }

}

// returns 1 on error, -1 if line skipped
int tokenize(char *line, struct Operation *op){
  // TODO add error checking
  char *token = strtok(line, " \n");
  if (token == NULL || token[0] == '/') {
    return -1;
  }
  
  // TODO Add LABEL, GOTO, etc...
  if(strcmp(token, "add") == 0 ||
     strcmp(token, "sub") == 0 ||
     strcmp(token, "neg") == 0 ||
     strcmp(token, "eq") == 0  ||
     strcmp(token, "gt") == 0  ||
     strcmp(token, "lt") == 0  ||
     strcmp(token, "and") == 0 ||
     strcmp(token, "or") == 0  ||
     strcmp(token, "not") == 0) {
    op->type = C_ARITHMETIC;
  } else if(strcmp(token, "push") == 0) {
    op->type = C_PUSH;
  } else if(strcmp(token, "pop") == 0) {
    op->type = C_POP;
  } else {
    fprintf(stderr, "Invalid token: %s\n", token);
    return(1);
  }

  if (op->type == C_ARITHMETIC) {
    strcpy(op->arg1, token);
    return 0;
  }

  token = strtok(NULL, " \n");
  if(token) {
    strcpy(op->arg1, token);
  }

  token = strtok(NULL, " \n");
  if(token) {
    op->arg2 = atoi(token);
    if(op->arg2 != 0 && token[0] == '0') {
      fprintf(stderr, "invalid arg2: %s\n", token);
      return 1;
    }
  }
  return 0;
}

int lex_file(FILE *fp, struct Operation *program) {
  // blank line and lines starting with // should be skipped
  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, fp)!= -1){
    // returns 1 on error, -1 if line skipped
    if(tokenize(line, program) != -1){
      //      print_op(*program);
      program++;
    }
  }
  free(line);
  return 0;
}

int main(int argc, char *argv[]){
  int16_t ram[RAM_SZ] = {0};
  struct Operation program[ROM_SZ] = {0};
  FILE *fp = NULL;
  int errnum; 

  if (argc < 2) {
    usage();
    exit(1);
  }

  fp = fopen(argv[1], "r");

  if(fp == NULL) {
    perror("Couldn't open file");
  }

  lex_file(fp, program);
      
  return 0;
}
