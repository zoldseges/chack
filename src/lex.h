#ifndef __LEX_H__
#define __LEX_H__

#include "types.h"

int parse_classes(parsed_classes *classes, char *input);
int build_vm_from_classes(VM *vm, parsed_classes *classes);

#endif /* __LEX_H__ */
