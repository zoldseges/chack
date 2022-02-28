#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "types.h"

// reconstruct
void *display_routine(void *vm_raw){
  int *run = (int *)&((VM *)vm_raw)->run;
  uint16_t *ram = ((VM *)vm_raw)->ram;
  struct io *io = (struct io *)&((VM *)vm_raw)->io;
  char  *fbp = io->fbp;
  int a = 0;
  while(*run) {
    a++;
    int x = 10;
    int y = 100;
    int col = 100;
    for(int i = 512; i < 768; i++){
      x = 10;
      for(int j = 0; j < 32; j++){
	uint16_t byte = ram[i*32 + j];
	for(int k = 0; k < 16; k++) {
	  long int location = (x+io->vinfo.xoffset) *
	    (io->vinfo.bits_per_pixel/8) +
	    (y+io->vinfo.yoffset) * io->finfo.line_length;
	  if(byte & 1){
	    *(fbp + location) = (int)(512-x/3);        // Some blue
	    *(fbp + location + 1) = 0;     // A little green
	    *(fbp + location + 2) = (int)x/2.2;    // A lot of red
	    *(fbp + location + 3) = 0;      // No transparency
	    location += 4;
	  } else {
	    *(fbp + location) = 0;        // Some blue
	    *(fbp + location + 1) = (int)x/2.3;     // A little green
	    *(fbp + location + 2) = 200-(y-100)/5;    // A lot of red
	    *(fbp + location + 3) = 0;      // No transparency
	    location += 4;
	  }
	  byte = byte >> 1;
	  x++;
	}
      }
      y++;
    }
    usleep(15000);
  }
  // clear
  
  int x = 10;
  int y = 100;
  for(int i = 512; i < 768; i++){
    x = 10;
    for(int j = 0; j < 32; j++){
      for(int k = 0; k < 16; k++) {
	long int location = (x+io->vinfo.xoffset) * (io->vinfo.bits_per_pixel/8) +
	  (y+io->vinfo.yoffset) * io->finfo.line_length;
	  *(fbp + location + 0) = 0;        // Some blue
	  *(fbp + location + 1) = 0;     // A little green
	  *(fbp + location + 2) = 0;    // A lot of red
	  *(fbp + location + 3) = 0;      // No transparency
	  location += 4;
	  x++;
      }
    }
    y++;
  }
}

int set_term(struct io *io){
  tcgetattr( STDIN_FILENO, &io->oldt);
  io->newt = io->oldt;
  io->newt.c_lflag &= ~(ICANON);
  io->newt.c_lflag &= ~(ECHO);
  tcsetattr( STDIN_FILENO, TCSANOW, &io->newt);
  fcntl(0, F_SETFL, O_NONBLOCK);
  return 0;
};

int reset_term(struct io *io){
    tcsetattr(STDIN_FILENO, TCSANOW, &io->oldt);
    return 0;
}

int set_fb(struct io *io){
  // Open the file for reading and writing
  io->fbfd = open("/dev/fb0", O_RDWR);
  if (io->fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }

  // Get fixed screen information
  if (ioctl(io->fbfd, FBIOGET_FSCREENINFO, &io->finfo) == -1) {
    perror("Error reading fixed information");
    exit(2);
  }

  // Get variable screen information
  if (ioctl(io->fbfd, FBIOGET_VSCREENINFO, &io->vinfo) == -1) {
    perror("Error reading variable information");
    exit(3);
  }

  // Figure out the size of the screen in bytes
  io->screensize = io->vinfo.xres * io->vinfo.yres * io->vinfo.bits_per_pixel / 8;
  
  // Map the device to memory
  io->fbp = (char *)mmap(0, io->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, io->fbfd, 0);
  if (io->fbp == MAP_FAILED) {
    perror("Error: failed to map framebuffer device to memory");
    exit(4);
  }

  return 0;
}

int connect_screen(VM *vm){
  set_fb(&vm->io);
  pthread_create(&vm->io.dp_tid, NULL, display_routine, (void *)vm);
  return 0;
}

int disconnect_screen(VM *vm){
  pthread_join(vm->io.dp_tid, NULL);
  munmap(vm->io.fbp, vm->io.screensize);
  close(vm->io.fbfd);
  return 0;
}

void *keyboard_routine(void *vm_raw){
  VM *vm = (VM *)vm_raw;
  int c = 0;
  int flags = fcntl(0, F_GETFL);
  while(vm->run) {
    c = getchar();
    if(c >= 32 && c < 128){
      vm->ram[KBD] = c;
    } else if (c == 27) {
      fcntl(0, F_SETFL, O_NONBLOCK);
      switch(c = getchar()){
      case 91:
	switch(c = getchar()){
	case 68:
	  vm->ram[KBD] = 130;
	  break;
	case 65:
	  vm->ram[KBD] = 131;
	  break;
	case 67:
	  vm->ram[KBD] = 132;
	  break;
	case 66:
	  vm->ram[KBD] = 133;
	  break;
	case 72:
	  vm->ram[KBD] = 134;
	  break;
	case 52 :
	  switch(c = getchar()){
	  case 126:
	    vm->ram[KBD] = 135;
	    break;
	  case 104:
	    vm->ram[KBD] = 138;
	    break;
	  default:
	    while((c = getchar()) != -1);
	    break;
	  }
	  break;
	case 53:
	  c = getchar();
	  vm->ram[KBD] = 136;	  
	  break;
	case 54:
	  c = getchar();
	  vm->ram[KBD] = 137;
	  break;
	case 80:
	  vm->ram[KBD] = 139;
	  break;
	case 49:
	  switch(c = getchar()){
	  case 53:
	    c = getchar();
	    vm->ram[KBD] = 145;
	    break;
	  case 55:
	    c = getchar();
	    vm->ram[KBD] = 146;
	    break;
	  case 56:
	    c = getchar();
	    vm->ram[KBD] = 147;
	    break;
	  case 57:
	    c = getchar();
	    vm->ram[KBD] = 148;
	    break;
	  default:
	    while((c = getchar()) != -1);
	    break;
	  }
	  break;
	case 50:
	  switch(c = getchar()){
	  case 48:
	    c = getchar();
	    vm->ram[KBD] = 149;
	    break;
	  case 49:
	    c = getchar();
	    vm->ram[KBD] = 150;
	    break;
	  case 51:
	    c = getchar();
	    vm->ram[KBD] = 151;
	    break;
	  case 52:
	    c = getchar();
	    vm->ram[KBD] = 152;
	    break;
	  default:
	    while((c = getchar()) != -1);
	    break;
	  }
	  break;
	default:
	  while((c = getchar()) != -1);
	  break;
	}
	break;
      case 79:
	switch(c = getchar()){
	case 80:
	  vm->ram[KBD] = 141;
	  break;
	case 81:
	  vm->ram[KBD] = 142;
	  break;
	case 82:
	  vm->ram[KBD] = 143;
	  break;
	case 83:
	  vm->ram[KBD] = 144;
	  break;
	default:
	  while((c = getchar()) != -1);
	  break;
	}
	break;
      case -1:
	vm->ram[KBD] = 140;
	break;
      default:
	while((c = getchar()) != -1);
	break;
      }
      fcntl(0, F_SETFL, flags);
    } else {
      switch(c){
      case 10:
	vm->ram[KBD] = 128;
	break;
      case 8:
	vm->ram[KBD] = 129;
	break;
      default:
	fcntl(0, F_SETFL, O_NONBLOCK);
	while((c = getchar()) != -1);
	fcntl(0, F_SETFL, flags);
	break;
      }
    }
    usleep(50000);
    vm->ram[KBD] = 0;
  }
  return 0;
}

int connect_kb(VM *vm){
  pthread_create(&vm->io.kb_tid, NULL, keyboard_routine, (void *)vm);
  return 0;
}

int disconnect_kb(VM *vm){
  pthread_join(vm->io.kb_tid, NULL);
  return 0;
}

int connect_io(VM *vm){
  set_term(&vm->io);
  connect_screen(vm);
  connect_kb(vm);
  return 0;
}

int disconnect_io(VM *vm){
  disconnect_screen(vm);
  disconnect_kb(vm);
  reset_term(&vm->io);
  return 0;
}
