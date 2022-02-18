#include <sys/stat.h>
#include <stdio.h>

/*
  returns 0 on error
  1 if file
  2 if dir
*/
int is_file_or_dir(char *path) {
  struct stat statbuf;
  int res = 0;
  if (stat(path, &statbuf) != 0) return 0;
  res = S_ISREG(statbuf.st_mode);
  if(res) return 1;
  res = S_ISDIR(statbuf.st_mode);
  if(res) return 2;
  return res;
}
