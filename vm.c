#include "types.h"
#include "utils.h"

int exec_arithmetic(op *op, int16_t *ram){
  switch(op->arg1) {
  case A_ADD:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a+b;
    break;
  case A_SUB:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a-b;
    break;
  case A_NEG:
    int16_t a = ram[--ram[0]];
    ram[ram[0]++] = -a
    break;
  case A_EQ:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a == b;
    break;
  case A_GT:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a > b;
    break;
  case A_LT:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a < b;
    break;
  case A_AND:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a & b;
    break;
  case A_OR:
    int16_t a = ram[--ram[0]];
    int16_t b = ram[--ram[0]];
    ram[ram[0]++] = a & b;
    break;
  case A_NOT:
    int16_t a = ram[--ram[0]];
    ram[ram[0]++] = ~a;
    break;
  }
}

// return 0 on success
int step(VM *vm){
  op *op = &vm->prog[vm->pc];
  switch (vm->prog[vm->pc].cmd) {
  case C_ARITHMETIC:
    
    vm->pc++;
    break;
  case C_PUSH:
    vm->pc++;
    break;
  case C_POP:
    vm->pc++;
    break;
  default:
    char *msg = NULL;
    sprintf(msg, "CMD can't be executed: code %d, pc: %d\n",
	    op->cmd, vm->pc);
    unreachable_branch_error(char *msg, __FILE__, __LINE__);
    break;
  }
  return 0;
}
