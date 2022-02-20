#ifndef __UTILS_H__
#define __UTILS_H__

#include "types.h"

void unreachable_branch_error(char *msg, char *filename, int line);
void atoi_error(char *snum, int n, char *filename, int line);

void print_parsed_prog(parsed_classes *classes);
void print_vm_prog(VM *vm, parsed_classes *classes);

/*
  returns 0 on error
  1 if file
  2 if dir
*/
int is_file_or_dir(char *path);

/*
  fills classes fname and cname 
  returns class_count
*/
int get_class_paths(class *classes, char *input);

// returns 0 on success
int build_vm_from_fpath(VM *vm, char *input);

#endif /* __UTILS_H__ */
