#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"

int exec_arithmetic(op *op, int16_t *ram){
  int16_t a, b;
  switch(op->arg1) {
  case A_ADD:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a+b;
    break;
  case A_SUB:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a-b;
    break;
  case A_NEG:
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = -a;
    break;
  case A_EQ:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a == b ? -1 : 0;
    break;
  case A_GT:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a > b ? -1 : 0;
    break;
  case A_LT:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a < b ? -1 : 0;
    break;
  case A_AND:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a & b;
    break;
  case A_OR:
    b = ram[--ram[SP_P]];
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = a | b;
    break;
  case A_NOT:
    a = ram[--ram[SP_P]];
    ram[ram[SP_P]++] = ~a;
    break;
  }
  return 0;
}

int exec_push(op *op, int16_t *ram) {
  switch(op->arg1){
  case A_ARG:
    ram[ram[SP_P]++] = ram[ram[ARG_BASE_P] + op->arg2];
    break;
  case A_LCL:
    ram[ram[SP_P]++] = ram[ram[LCL_BASE_P] + op->arg2];
    break;
  case A_STATIC:
    fprintf(stderr, "&s:&d: Unimplemented\n", __FILE__, __LINE__);
    exit(1);
    break;
  case A_CONST:
    ram[ram[SP_P]++] = op->arg2;
    break;
  case A_THIS:
    ram[ram[SP_P]++] = ram[ram[THIS_BASE_P] + op->arg2];
    break;
  case A_THAT:
    ram[ram[SP_P]++] = ram[ram[THAT_BASE_P] + op->arg2];
    break;
  case A_POINTER:
    fprintf(stderr, "&s:&d: Unimplemented\n", __FILE__, __LINE__);
    exit(1);
    break;
  case A_TEMP:
    /* ERROR DETECTION */
    /*
      if(op->arg2 > 7) {
      fprintf(stderr, "ERROR\n");
      fprintfstderr, "%s:%d\n", __FILE__, __LINE__);
      fprintf(stderr, "Invalid temp address: %d\n", op->arg2);
}
     */
    ram[ram[SP_P]++] = ram[TEMP_BASE + op->arg2];
    break;
  default:
    char *msg = NULL;
    sprintf(msg, "Can't push segment: code %d\n",
	    op->arg1);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    break;
  }
  return 0;
}

int exec_pop(op *op, int16_t *ram) {
  int16_t val = ram[--ram[SP_P]];
  switch(op->arg1){
  case A_ARG:
    ram[ram[ARG_BASE_P] + op->arg2] = val;
    break;
  case A_LCL:
    ram[ram[LCL_BASE_P] + op->arg2] = val;
    break;
  case A_STATIC:
    fprintf(stderr, "&s:&d: Unimplemented\n", __FILE__, __LINE__);
    exit(1);
    break;
  case A_THIS:
    ram[ram[THIS_BASE_P] + op->arg2] = val;
    break;
  case A_THAT:
    ram[ram[THAT_BASE_P] + op->arg2] = val;
    break;
  case A_POINTER:
    fprintf(stderr, "&s:&d: Unimplemented\n", __FILE__, __LINE__);
    exit(1);
    break;
  case A_TEMP:
    /* ERROR DETECTION */
    /*
      if(op->arg2 > 7) {
      fprintf(stderr, "ERROR\n");
      fprintfstderr, "%s:%d\n", __FILE__, __LINE__);
      fprintf(stderr, "Invalid temp address: %d\n", op->arg2);
}
     */
    ram[TEMP_BASE + op->arg2] = val;
    break;
  default:
    char *msg = NULL;
    sprintf(msg, "Can't be poped into segment: code: %d\n",
	    op->arg1);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    break;
  }
  return 0;
}

// return 0 on success
int step(VM *vm){
  op *op = &vm->prog[vm->pc];
  int16_t *ram = vm->ram;
  switch (vm->prog[vm->pc].cmd) {
  case C_ARITHMETIC:
    exec_arithmetic(op, ram);
    vm->pc++;
    break;
  case C_PUSH:
    exec_push(op, ram);
    vm->pc++;
    break;
  case C_POP:
    exec_pop(op, ram);
    vm->pc++;
    break;
  default:
    char *msg = NULL;
    sprintf(msg, "CMD can't be executed: code: %d\n",
	    op->cmd);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    break;
  }
  return 0;
}
