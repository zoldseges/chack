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
  int prog_end;
} class;

int lex_file(class *class) {
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
  fclose(fp);
  return prog_line;
}

void print_parsed_class(parsed_op prog_parsed[], int len){
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
  char class[64];
  char func[64];
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

// return class_count
int get_class_paths(class *classes, char *input) {
  int count = 0;
  int file_or_dir = is_file_or_dir(input);
  char path[256];
  if (file_or_dir == 1) {
    count = 1;
    strcpy(classes[0].fpath, input);
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

int main(int argc, char *argv[]){
  int16_t ram[RAM_SZ] = {0};
  class *parsed_classes = NULL;
  int class_count = 0;
  int errnum; 
  char *input = NULL;

  ref ref_table[16384] = {0};
  int ref_table_end = 0;

  // no input given
  if (argc < 2) {
    exit(1);
  } else {
    input = argv[1];
  }

  parsed_classes = malloc(sizeof(class) * 64);
  class_count = get_class_paths(parsed_classes, input);
  for(int i = 0; i < class_count; i++){
    printf("%-16s %s\n", parsed_classes[i].fpath, parsed_classes[i].cname);
  }
  /* prog_end = lex_file(fp, prog_parsed); */
  /* build_ref_table(ref_table, &ref_table_end, prog_parsed, prog_end); */
  /* for(int i = 0; i < ref_table_end; i++){ */
  /*   printf("%d %s\n", ref_table[i].addr, ref_table[i].arg); */
  /* } */
  
  free(parsed_classes);
  return 0;
}
