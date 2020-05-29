#include <libNVMlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

LIB_TOID_DECLARE(int);

// Merges two subarrays of arr[]. 
// First subarray is arr[l..m] 
// Second subarray is arr[m+1..r] 
// Inplace Implementation 
void merge(LIB_TOID(int) arr, int start, int mid, int end) 
{ 
	int start2 = mid + 1; 

	// If the direct merge is already sorted 
	if (LIB_D_RO(arr)[mid] <= LIB_D_RO(arr)[start2]) { 
		return; 
	} 

	// Two pointers to maintain start 
	// of both arrays to merge 
	while (start <= mid && start2 <= end) { 

		// If element 1 is in right place 
		if (LIB_D_RO(arr)[start] <= LIB_D_RO(arr)[start2]) { 
			start++; 
		} 
		else { 
			int value = LIB_D_RO(arr)[start2]; 
			int index = start2; 

			// Shift all the elements between element 1 
			// element 2, right by 1. 
			while (index != start) {
				 LIB_D_RW(arr)[index] = LIB_D_RW(arr)[index - 1]; 
				index--; 
			} 
			 LIB_D_RW(arr)[start] = value; 

			// Update all the pointers 
			start++; 
			mid++; 
			start2++; 
		} 
	} 
} 

/* l is for left index and r is right index of the 
sub-array of arr to be sorted */
void mergeSort(LIB_TOID(int) arr, int l, int r) 
{ 
	if (l < r) { 

		// Same as (l + r) / 2, but avoids overflow 
		// for large l and r 
		int m = l + (r - l) / 2; 

		// Sort first and second halves 
		mergeSort(arr, l, m); 
		mergeSort(arr, m + 1, r); 

		merge(arr, l, m, r); 
	} 
} 

/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(LIB_TOID(int) A, int size) 
{ 
	int i; 
	for (i = 0; i < size; i++) 
		printf("%d ", LIB_D_RO(A)[i]); 
	printf("\n"); 
} 

/* Driver program to test above functions */
int main() 
{ 
    initialize();
    srand(time(NULL));
    clock_t end, start;
    start = clock();
    int arr_size = 1000;
    LIB_TOID(int) arr = (LIB_TOID(int))memalloc(arr_size * sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) flag = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    LIB_TOID(int) idx = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);
    // LIB_TOID(int) idy = (LIB_TOID(int))memalloc(sizeof(int), NVRAM_HEAP);

    int i =  *LIB_D_RO(idx) < arr_size ? *LIB_D_RO(idx):0;
    if(*LIB_D_RO(flag) == 1) {
        printf("Initialization SKipped. \n");
        goto skip_init;
    }

    for(int i = 0; i< arr_size; i++) {
        LIB_D_RW(arr)[i] = rand()%(arr_size * 2);
    }

    *LIB_D_RW(flag) = 1;

skip_init:

	printArray(arr, arr_size); 
	mergeSort(arr, 0, arr_size - 1); 
	printArray(arr, arr_size); 

    end = clock();
    printf("Elapsed time = %f\n", (double)(end - start)/CLOCKS_PER_SEC);

    // memfree(arr);
    // memfree(flag);
    // memfree(idx);

	return 0; 
} 
