#include<stdio.h>
#include<stdlib.h>
#include "test.h"
const int NUMBER_OF_CHUNKS = 9;
int *isUsed;                // array of if each chunk is used, 
// 0 - unused, 1 - used and has internal fragmentation, 2 - completely used
int **basePointersArray;  // array of pointers that keeps track of the base pointer for each chunk
int **endPointersArray;   // array of pointers that keeps track of the end pointer for each chunk `

void split_chunks(int, int*);

// min allocation 1024
// 1048576   512k   256k    128k    64k     32k       
// 1024  =   512     256     128     64      32      16      8       4       4

// 1024 0-599, 600-799
int main()
{
    int n = 1024;
    // printf("enter heap size: ");
    // scanf("%d", &n);

    // setting isUsed array to 0
    isUsed = malloc(NUMBER_OF_CHUNKS * sizeof(int));
    for(int i=0;i<NUMBER_OF_CHUNKS;i++){isUsed[i] = 0;}

    // allocating the initial heap
    int *basePointer = malloc((n * sizeof(int)));
    printf("base pointer of entire heap: %p\n", basePointer);   // start of our allocated heap
    printf("last pointer of entire heap: %p\n", basePointer+(n-1));    // end of our allocated heap   
    printf("\n");

    basePointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(int*));
    endPointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(int*));
    split_chunks(n, basePointer);
    
}
void split_chunks(int heapSize, int *basePointer)
{
    // if for example heapSize is 1024 then chunk sizes would be
    // 1024  =   512     256     128     64      32      16      8       4       4
    // using array so irrespective of user input for heap size it would still give valid chunk addresses
    int sizes[] = {heapSize / 2, heapSize/4, heapSize/8, heapSize/16, heapSize/32, heapSize/64, heapSize/128, heapSize/256, heapSize/256};
    // chunk 1 addresses
    basePointersArray[0] = basePointer;
    endPointersArray[0] = basePointer + (sizes[0] - 1);
    printf("chunk 1 end pointer: %p\n", endPointersArray[0]);

    // assigning chunks 2-9 addresses
    for(int i=1;i<NUMBER_OF_CHUNKS;i++)
    {
        basePointersArray[i] = endPointersArray[i-1] + 1;
        endPointersArray[i] = basePointersArray[i] + (sizes[i] - 1);
    }

    for(int i=0;i<NUMBER_OF_CHUNKS; i++)
    {
        printf("base pointer for %dth chunk: ", i);
        printf("%p\n", basePointersArray[i]);
        printf("end pointer for %dth chunk: ", i);
        printf("%p\n", endPointersArray[i]);
        printf("\n");
    }
}
void freeUpInitialHeap(int *basePointer)
{
    free(basePointer);
    basePointer = NULL;
}