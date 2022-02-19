#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include "config.h"

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

struct ref {
  char func[64];
  char arg[64];
  uint16_t addr;
};

typedef struct ref_tbl {
  struct ref tbl[ROM_SZ/2];
  int tbl_sz;
} ref_tbl;

typedef struct parsed_classes {
  class classes[64];
  int class_count;
  ref_tbl ref_tbl;
} parsed_classes;

typedef struct encoded_op {
  enum CMD cmd;
  uint16_t arg1;
  uint16_t arg2;
} op;

typedef struct VM {
  int16_t ram[RAM_SZ];
  op prog[ROM_SZ];
  int pc;
  int prog_lines;
} VM;

#endif /* __TYPES_H__ */
