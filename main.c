#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define RAM_SZ 32768
#define ROM_SZ 32768

extern int errno;

typedef struct parsed_op {
  char cmd[32];
  char arg1[64];
  char arg2[32];
} parsed_op;

int lex_file(FILE *fp, parsed_op *op) {
  char *line = NULL;
  size_t len = 0;
  int prog_line = 0;
  char *token = NULL;
  parsed_op *curr = NULL;

  while(getline(&line, &len, fp) != -1){
    token = strtok(line, " \n");
    // skip comments and empty lines
    if (token == NULL || token[0] == '/') continue;

    curr = op;
    op++;
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
  return prog_line;
}

void print_parsed_prog(parsed_op prog_parsed[], int len){
  for(int i = 0; i < len; i++){
    parsed_op op = prog_parsed[i];
    printf("%s", op.cmd);
    if (*op.arg1) printf(" %s", op.arg1);
    if (*op.arg2) printf(" %s", op.arg2);
    printf("\n");
  }
}

/*
  operation encoding
*/
enum CMD {
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

typedef struct encoded_op {
  enum CMD cmd;
  uint16_t arg1;
  uint16_t arg2;
} op;

typedef struct ref {
  char arg[64];
  uint16_t addr;
} ref;

void build_ref_table(ref *ref_table,
		     int *ref_table_end,
		     parsed_op *op,
		     int prog_end) {
  ref *ref = ref_table;
  for(int addr = 0; addr < prog_end; addr++) {
    parsed_op *curr_op = &op[addr];
    if (strcmp(curr_op->cmd, "label") == 0 ||
	strcmp(curr_op->cmd, "function") == 0) {
      strcpy(ref->arg, curr_op->arg1);
      ref->addr = addr;
      (*ref_table_end)++;
      ref = ref_table + *ref_table_end;
    }
  }
}

int main(int argc, char *argv[]){
  int16_t ram[RAM_SZ] = {0};
  parsed_op prog_parsed[ROM_SZ] = {0};
  FILE *fp = NULL;
  int prog_end = 0;
  int errnum; 

  ref ref_table[16384] = {0};
  int ref_table_end = 0;

  // no input given
  if (argc < 2) {
    exit(1);
  }

  fp = fopen(argv[1], "r");

  if(fp == NULL) {
    perror("Couldn't open file");
  }

  prog_end = lex_file(fp, prog_parsed);
  /* print_parsed_prog(prog_parsed, prog_end); */
  build_ref_table(ref_table, &ref_table_end, prog_parsed, prog_end);
  for(int i = 0; i < ref_table_end; i++){
    printf("%d %s\n", ref_table[i].addr, ref_table[i].arg);
  }
  return 0;
}
