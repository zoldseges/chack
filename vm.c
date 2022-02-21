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
  case S_ARG:
    ram[ram[SP_P]++] = ram[ram[ARG_BASE_P] + op->arg2];
    break;
  case S_LCL:
    ram[ram[SP_P]++] = ram[ram[LCL_BASE_P] + op->arg2];
    break;
  case S_STATIC:
    ram[ram[SP_P]++] = ram[op->arg2];
    break;
  case S_CONST:
    ram[ram[SP_P]++] = op->arg2;
    break;
  case S_THIS:
    ram[ram[SP_P]++] = ram[ram[THIS_BASE_P] + op->arg2];
    break;
  case S_THAT:
    ram[ram[SP_P]++] = ram[ram[THAT_BASE_P] + op->arg2];
    break;
  case S_POINTER:
    if (op->arg2 == 0) {
      ram[ram[SP_P]++] = ram[THIS_BASE_P];
    } else if (op->arg2 == 1){
      ram[ram[SP_P]++] = ram[THAT_BASE_P];
    } else {
    char msg[128] = {0};
    sprintf(msg, "Invalid pointer argument: code %d\n",
	    op->arg2);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    }
    break;
  case S_TEMP:
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
    char msg[128] = {0};
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
  case S_ARG:
    ram[ram[ARG_BASE_P] + op->arg2] = val;
    break;
  case S_LCL:
    ram[ram[LCL_BASE_P] + op->arg2] = val;
    break;
  case S_STATIC:
    ram[op->arg2] = val;
    break;
  case S_THIS:
    ram[ram[THIS_BASE_P] + op->arg2] = val;
    break;
  case S_THAT:
    ram[ram[THAT_BASE_P] + op->arg2] = val;
    break;
  case S_POINTER:
    if (op->arg2 == 0) {
      ram[THIS_BASE_P] = val;
    } else if (op->arg2 == 1){
      ram[THAT_BASE_P] = val;
    } else {
    char msg[128] = {0};
    sprintf(msg, "Invalid pointer argument: code %d\n",
	    op->arg2);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    }
    break;
  case S_TEMP:
    /* ERROR DETECTION */
    
    if(op->arg2 > 7) {
      fprintf(stderr, "ERROR\n");
      fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
      fprintf(stderr, "Invalid temp address: %d\n", op->arg2);
    }
    ram[TEMP_BASE + op->arg2] = val;
    break;
  default:
    char msg[128] = {0};
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
  case C_LABEL:
    vm->pc++;
    break;
  case C_GOTO:
    vm->pc = op->arg1;
    vm->pc++;
    break;
  case C_IF:
    int16_t cond = ram[--ram[SP_P]];
    if( cond != 0 ){
      vm->pc = op->arg1;
    }
    vm->pc++;
    break;
  case C_FUNCTION:
    for(int i = 0; i < op->arg2; i++){
      ram[ram[SP_P]++] = 0;
    }
    vm->pc++;
    break;
  case C_CALL:
    ram[ram[SP_P]++] = vm->pc + 1;
    ram[ram[SP_P]++] = ram[LCL_BASE_P];
    ram[ram[SP_P]++] = ram[ARG_BASE_P];
    ram[ram[SP_P]++] = ram[THIS_BASE_P];
    ram[ram[SP_P]++] = ram[THAT_BASE_P];
    ram[ARG_BASE_P] = ram[SP_P] - 5 - op->arg2;
    ram[LCL_BASE_P] = ram[SP_P];
    vm->pc = op->arg1;
    break;
  case C_RETURN:
    uint16_t frame		= ram[LCL_BASE_P];
    uint16_t ret_addr		= ram[frame - 5];
    ram[ram[ARG_BASE_P]]	= ram[--ram[SP_P]];
    ram[SP_P]			= ram[ARG_BASE_P] + 1;
    ram[THAT_BASE_P]		= ram[frame - 1];
    ram[THIS_BASE_P]		= ram[frame - 2];
    ram[ARG_BASE_P]		= ram[frame - 3];
    ram[LCL_BASE_P]		= ram[frame - 4];
    vm->pc = ret_addr;
    break;
  default:
    char msg[128] = {0};
    sprintf(msg, "CMD can't be executed: code: %d\n",
	    op->cmd);
    unreachable_branch_error(msg, __FILE__, __LINE__);
    break;
  }
  return 0;
}

void run(VM *vm){
  
  while(vm->pc < vm->prog_lines){
    step(vm);
    // debug
    /* printf("%d %d\n",0, vm->ram[0]); */
    /* printf("%d %d\n",vm->ram[0] - 1, vm->ram[vm->ram[0]-1]); */
    /* printf("-----------\n"); */
    // end debug
  }
}
