#include<stdio.h>
#include<string.h>

int assign_static(char *, char *);

int main() {
  char *input[] = {
    "A1", "2", // 16
    "A1", "2", // 16
    "B1", "0", // 17
    "A1", "1", // 18
    "C1", "3", // 19
    "B1", "0", // 17
    "A2", "2", // 20
    "A1", "1", // 18
    "A1", "2", // 16
    "END",
  };
  
  for(int i = 0; strcmp(input[i], "END") != 0; i += 2){
    printf("%d\n", assign_static(input[i], input[i+1]));
  }
  assign_static("free", "free");
  return 0;
}
