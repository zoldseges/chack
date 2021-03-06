#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <linux/fb.h>
#include <termios.h>
#include <pthread.h>
#include "config.h"

#define SP_P		0
#define LCL_BASE_P	1
#define ARG_BASE_P	2
#define THIS_BASE_P	3
#define THAT_BASE_P	4
#define TEMP_BASE	5
#define STATIC_BASE_P   16

struct io {
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  char *fbp;
  int fbfd;
  long screensize;
  struct termios oldt;
  struct termios newt;

  pthread_t dp_tid;
  pthread_t kb_tid;
};

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

enum S_SEGMENT {
  S_ARG,
  S_LCL,
  S_STATIC,
  S_CONST,
  S_THIS,
  S_THAT,
  S_POINTER,
  S_TEMP
};
  
struct ref {
  char *class;
  char *func;
  char *arg1;
  char *arg2;
  int16_t addr;
};

typedef struct ref_tbl {
  struct ref tbl[ROM_SZ/2];
  int tbl_sz;
} ref_tbl;

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

typedef struct parsed_classes {
  class classes[64];
  int class_count;
  ref_tbl ref_tbl;
} parsed_classes;

typedef struct encoded_op {
  enum CMD cmd;
  int16_t arg1;
  int16_t arg2;
} op;

typedef struct VM {
  int16_t ram[RAM_SZ];
  op prog[ROM_SZ];
  int pc;
  int prog_lines;
  struct io io;
  int run;
} VM;

#endif /* __TYPES_H__ */
