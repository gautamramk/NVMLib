// C program for implementation of Bubble sort 
#include <stdio.h> 
#include <time.h> 
#include <malloc.h>
#include <stdlib.h>

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 
  
// A function to implement bubble sort 
void bubbleSort(int *arr, int n) 
{ 
   int i, j; 
   for (i = 0; i < n-1; i++)       
  
       // Last i elements are already in place    
       for (j = 0; j < n-i-1; j++)  
           if (arr[j] > arr[j+1]) 
              swap(&arr[j], &arr[j+1]); 
} 
  
/* Function to print an array */
void printArray(int arr[], int size) 
{ 
    int i; 
    for (i=0; i < size; i++) 
        printf("%d ", arr[i]); 
    printf("\n"); 
} 
  
// Driver program to test above functions 
int main() 
{ 
    srand(time(NULL));
    clock_t end, start;
    start = clock();
    int size = 10000;
    int* arr = (int*)malloc(size*sizeof(int));
    for(int i = 0; i < size; i++){
        arr[i] = rand()%100000;
    }

    bubbleSort(arr, size);
    end = clock();
    printf("Elapsed time = %f", (double)(end - start)/CLOCKS_PER_SEC); 
    //printf("Sorted array: \n"); 
    //printArray(arr, size); 
    return 0; 
} 