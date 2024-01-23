#include "stdio.h"
#include "unistd.h" // Include the header for sleep function


int main(){
 for (int i = 0; i < 20; i++){
    printf("%d\n", i);
    sleep(1);
 } 
}