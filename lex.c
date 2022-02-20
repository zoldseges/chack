#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"

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

/*
  building VM
*/

/*
 returns given address of static variable
*/

// resets if class and arg2 == NULL
uint16_t assign_static(char *class, char *arg2) {
  static char *store[ROM_SZ] = {NULL};
  static int store_end = 0;
  if (class == NULL && arg2 == NULL){
    for(int i = 0; i < store_end; i++){
      store[i] = NULL;
    }
    store_end = 0;
    return -1;
  }
  int addr = STATIC_BASE_P;
  char *curr_class = NULL;
  char *curr_arg2 = NULL;
  for(int i = 0; i < store_end; i+=2){
    curr_class = store[i];
    curr_arg2 = store[i+1];
    if((strcmp(curr_class, class) == 0) &&
       (strcmp(curr_arg2, arg2) == 0)){
      return addr;
    }
    addr++;
  }
  
  if(addr + STATIC_BASE_P >= 256){
    fprintf(stderr, "ERROR\n");
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
    fprintf(stderr, "Static segment filled\n");
    exit(1);
  }
  
  store[store_end++] = class;
  store[store_end++] = arg2;
  return addr;
}

void build_ref_tbl(parsed_classes *classes) {
  char  *func		= NULL;
  int	addr		= 0;
  int	is_label	= 0;
  int	is_function	= 0;
  int	is_static	= 0;
  classes->ref_tbl.tbl_sz = 0;

  // reset static store
  assign_static(NULL, NULL);
  struct ref *curr_ref = classes->ref_tbl.tbl;
  for(int i = 0; i < classes->class_count; i++){
    for(int j = 0; j < classes->classes[i].prog_lines; j++){
      parsed_op *op = &classes->classes[i].prog[j];
      is_label	  = !strcmp(op->cmd, "label");
      is_function = !strcmp(op->cmd, "function");

      if (is_label || is_function) {
	if (is_function) {
	  func = op->arg1;
	}
	curr_ref->class = classes->classes[i].cname;
	curr_ref->func	= func;
	curr_ref->arg1	= op->arg1;
	curr_ref->arg2  = is_function ? op->arg2 : "-1";
	curr_ref->addr	= addr;
	classes->ref_tbl.tbl_sz++;
	curr_ref = &classes->ref_tbl.tbl[classes->ref_tbl.tbl_sz];
      }
      is_static	  = ( ( (strcmp(op->cmd, "push") == 0) ||
		     (strcmp(op->cmd, "pop") == 0) ) &&
		     (strcmp(op->arg1, "static") == 0) );
      if (is_static) {
	curr_ref->class = classes->classes[i].cname;
	curr_ref->func	= func;
	curr_ref->arg1	= op->arg1;
	curr_ref->arg2	= op->arg2;
	curr_ref->addr  = assign_static(curr_ref->class, curr_ref->arg2);
	classes->ref_tbl.tbl_sz++;
	curr_ref = &classes->ref_tbl.tbl[classes->ref_tbl.tbl_sz];
      }
      addr++;
    }
  }
}

/*
  virtual machine
*/

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
    char msg[128] = {0};
    sprintf(msg, "Unreachable branch. cmd: %s arg1: %s arg2: %s\n",
	    p_op->cmd, p_op->arg1, p_op->arg2);
    unreachable_branch_error(msg, __FILE__, __LINE__);
  }
}

void encode_pushpop(parsed_op *p_op, op *e_op){
  if (strcmp(p_op->arg1, "argument") == 0) {
    e_op->arg1 = S_ARG;
  } else if (strcmp(p_op->arg1, "local") == 0) {
    e_op->arg1 = S_LCL;
  } else if (strcmp(p_op->arg1, "static") == 0) {
    e_op->arg1 = S_STATIC;
  } else if (strcmp(p_op->arg1, "constant") == 0) {
    e_op->arg1 = S_CONST;
  } else if (strcmp(p_op->arg1, "this") == 0) {
    e_op->arg1 = S_THIS;
  } else if (strcmp(p_op->arg1, "that") == 0) {
    e_op->arg1 = S_THAT;
  } else if (strcmp(p_op->arg1, "pointer") == 0) {
    e_op->arg1 = S_POINTER;
  } else if (strcmp(p_op->arg1, "temp") == 0) {
    e_op->arg1 = S_TEMP;
  } else {
    char msg[128] = {0};
    sprintf(msg, "Unreachable branch. cmd: %s arg1: %s arg2: %s\n",
	    p_op->cmd, p_op->arg1, p_op->arg2);
    unreachable_branch_error(msg, __FILE__, __LINE__);
  }
}

void encode_ref(parsed_op *p_op, op *e_op, ref_tbl *ref_tbl, char *curr_func){
  for(int i = 0; i < ref_tbl->tbl_sz; i++){
    if( strcmp(ref_tbl->tbl[i].func, curr_func) == 0 &&
	strcmp(ref_tbl->tbl[i].arg1, p_op->arg1) == 0){
      e_op->arg1 = ref_tbl->tbl[i].addr;
      return;
    }
  }
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
  fprintf(stderr, "Reference can't be resolved: %s %s %s\n", p_op->cmd, p_op->arg1, p_op->arg2);
  exit(1);
}



// looks up static variable argument in reftable and returns its static address
uint16_t encode_static_var_arg(char *cname, char *arg2, ref_tbl *ref_tbl) {
  uint16_t addr = 0;
  for(int i = 0; i < ref_tbl->tbl_sz; i++){
    if((strcmp(ref_tbl->tbl[i].class, cname) == 0) &&
       (strcmp(ref_tbl->tbl[i].arg2, arg2) == 0)) {
      return ref_tbl->tbl[i].addr;
    }
  }
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
  fprintf(stderr, "Static variable reference can't be resolved: class: %s arg2: %s\n", cname, arg2);
  exit(1);
}

void encode_cmd(parsed_op *p_op,
		op *e_op,
		ref_tbl *ref_tbl,
		char *curr_func,
		char *curr_class){
  uint16_t arg2 = 0;
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
    encode_pushpop(p_op, e_op);
    if(e_op->arg1 == S_STATIC){
      arg2 = encode_static_var_arg(curr_class, p_op->arg2, ref_tbl);
    } else {
      arg2 = atoi(p_op->arg2);
      atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    }
    e_op->arg2 = arg2;
  } else if (strcmp(p_op->cmd, "pop") == 0) {
    e_op->cmd = C_POP;
    encode_pushpop(p_op, e_op);
    if(e_op->arg1 == S_STATIC){
      arg2 = encode_static_var_arg(curr_class, p_op->arg2, ref_tbl);
    } else {
      arg2 = atoi(p_op->arg2);
      atoi_error(p_op->arg2, arg2, __FILE__, __LINE__);
    }
    e_op->arg2 = arg2;
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
    // TODO cp pointer instead of string lex.c:282
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
    char msg[128] = {0};
    sprintf(msg, "Unreachable branch. cmd: %s arg1: %s arg2: %s\n",
	    p_op->cmd, p_op->arg1, p_op->arg2);
    unreachable_branch_error(msg, __FILE__, __LINE__);
  }
}

void __build_vm(VM *vm, ref_tbl *ref_tbl, class *classes, int class_count) {
  vm->prog_lines = 0;
  char curr_func[64] = {0};
  for(int i = 0; i < class_count; i++){
    // debug
    /* printf("----  %-12s----\n", classes[i].cname); */
    for(int j = 0; j < classes[i].prog_lines; j++){
      // debug
      /* printf("%-12s %-32s %-8s\n", classes[i].prog[j].cmd, */
      /* 	     classes[i].prog[j].arg1, classes[i].prog[j].arg2); */
      // debug
      /* printf("%8d %s\n", j, ref_tbl->tbl[0].func); */
      encode_cmd(&classes[i].prog[j],
		 &vm->prog[vm->prog_lines],
		 ref_tbl,
		 curr_func,
		 classes[i].cname);
      vm->prog_lines++;
    }
  }
}

//retruns 0 on succes
int parse_classes(parsed_classes *classes, char *input){
  int class_count = 0;
  classes->class_count = get_class_paths(classes->classes, input);
  for(int i = 0; i < classes->class_count; i++){
    lex(&classes->classes[i]);
  }
  build_ref_tbl(classes);
  // debug
  /* for(int i = 0; i < classes->ref_tbl.tbl_sz; i++){ */
  /*   struct ref *ref = &classes->ref_tbl.tbl[i]; */
  /*   printf("%-12s %-32s %-32s %-8s %d\n", ref->class, */
  /* 	   ref->func, ref->arg1, ref->arg2, ref->addr); */
	   
  /* } */
  
  return 0;
}

// returns 0 on success
int build_vm_from_classes(VM *vm, parsed_classes *classes){
  for(int i = 0; i < classes->class_count; i++){
    lex(&classes->classes[i]);
  }
  __build_vm(vm, &classes->ref_tbl, classes->classes, classes->class_count);
  return 0;
}

