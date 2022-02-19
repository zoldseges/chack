#ifndef __UTILS_H__
#define __UTILS_H__

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

#endif /* __UTILS_H__ */
