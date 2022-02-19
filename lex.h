#ifndef __LEX_H__
#define __LEX_H__

int parse_classes(parsed_classes *classes, char *input);
int build_vm(VM *vm, parsed_classes *classes);

#endif /* __LEX_H__ */
