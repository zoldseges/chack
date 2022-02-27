#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "types.h"
#include <linux/fb.h>
#include <termios.h>

int connect_screen(VM *vm);

#endif /* __SCREEN_H__ */
