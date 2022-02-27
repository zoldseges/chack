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
  struct screen *scr = (struct screen *)&((VM *)vm_raw)->scr;
  char  *fbp = scr->fbp;
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
	  long int location = (x+scr->vinfo.xoffset) *
	    (scr->vinfo.bits_per_pixel/8) +
	    (y+scr->vinfo.yoffset) * scr->finfo.line_length;
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
	long int location = (x+scr->vinfo.xoffset) * (scr->vinfo.bits_per_pixel/8) +
	  (y+scr->vinfo.yoffset) * scr->finfo.line_length;
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

int set_term(struct screen *scr){
  tcgetattr( STDIN_FILENO, &scr->oldt);
  scr->newt = scr->oldt;
  scr->newt.c_lflag &= ~(ICANON);
  scr->newt.c_lflag &= ~(ECHO);
  tcsetattr( STDIN_FILENO, TCSANOW, &scr->newt);
  fcntl(0, F_SETFL, O_NONBLOCK);
  return 0;
};

int reset_term(struct screen *scr){
    tcsetattr(STDIN_FILENO, TCSANOW, &scr->oldt);
    return 0;
}

int set_fb(struct screen *scr){
  // Open the file for reading and writing
  scr->fbfd = open("/dev/fb0", O_RDWR);
  if (scr->fbfd == -1) {
    perror("Error: cannot open framebuffer device");
    exit(1);
  }

  // Get fixed screen information
  if (ioctl(scr->fbfd, FBIOGET_FSCREENINFO, &scr->finfo) == -1) {
    perror("Error reading fixed information");
    exit(2);
  }

  // Get variable screen information
  if (ioctl(scr->fbfd, FBIOGET_VSCREENINFO, &scr->vinfo) == -1) {
    perror("Error reading variable information");
    exit(3);
  }

  // Figure out the size of the screen in bytes
  scr->screensize = scr->vinfo.xres * scr->vinfo.yres * scr->vinfo.bits_per_pixel / 8;
  
  // Map the device to memory
  scr->fbp = (char *)mmap(0, scr->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, scr->fbfd, 0);
  if (scr->fbp == MAP_FAILED) {
    perror("Error: failed to map framebuffer device to memory");
    exit(4);
  }

  return 0;
}

int connect_screen(VM *vm){
  set_term(&vm->scr);
  set_fb(&vm->scr);
  pthread_create(&vm->scr.dp_tid, NULL, display_routine, (void *)vm);
  return 0;
}

int disc_screen(VM *vm){
  pthread_join(vm->scr.dp_tid, NULL);
  munmap(vm->scr.fbp, vm->scr.screensize);
  close(vm->scr.fbfd);
  reset_term(&vm->scr);
  return 0;
}
