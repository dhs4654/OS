#include <stdio.h> 
#include "bf_malloc.c" 
int main(int argc, char *argv[]) { 
  int request[10] = {8192,2400,16384,800,1800,512,2048,4096,1800,6000}; 
  int* arrays[10]; 
  int i, j, k; 
  for (k = 0; k < 2; k++) { 
    for (i = 0; i < 6; i++) { 
      arrays[i] = (int *)bf_malloc(sizeof(int) * request[i + 5 * k]); 
    } 
    for (i = 0; i < 6; i++) { 
      free(arrays[i]);
    } 
  } 
  return 0;
}