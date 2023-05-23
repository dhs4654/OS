#include <stdio.h>  
#include "ff_malloc.c"
int main(int argc, char *argv[]) {  
  int* arrays[50];  
  int i, j, k;
  for (k = 0; k < 3; k++) {  
    for (i = 0; i < 50; i++) {  
      arrays[i] = (int *)ff_malloc(sizeof(int) * (i + 1));  
      for (j = 0; j <= i; j++) {  
        arrays[i][j] = j;  
    printf("arrays [%d][%d] is OK.\n",i,j);  
      }  
    }
    for (i = 0; i < 50; i++) {  
      free(arrays[i]);  
    }  
  }
  return 0;  
}  