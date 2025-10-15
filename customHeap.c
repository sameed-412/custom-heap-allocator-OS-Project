#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
const int NUMBER_OF_CHUNKS = 9;
int *isUsed;                // array of if each chunk is used, 
// 0 - unused, 1 - used and has internal fragmentation, 2 - completely used
int **basePointersArray;  // array of pointers that keeps track of the base pointer for each chunk
int **endPointersArray;   // array of pointers that keeps track of the end pointer for each chunk `

typedef struct {
    char name[5];         // p0, p1,....,pn-1
    int size;             
    int allocatedChunk;   // which chunk it was allocated to, set to -1 if not allocated)
} Process;

void split_chunks(int, int*);
void freeInitialHeap(int *basePointer);
void processInput(Process *processes, int *processCount, int heapSize);
void resetIsUsed();
void firstFit(Process *processes, int numProcesses, int *chunkSizes);
void bestFit(Process *processes, int numProcesses, int *chunkSizes);
void worstFit(Process *processes, int numProcesses, int *chunkSizes);
float calculateUtilization(Process *processes, int numProcesses, int heapSize);
void compareAlgorithms(Process *originalProcesses, int numProcesses, int *chunkSizes, int heapSize);


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

    // process input
    Process processes[20];
    int numProcesses;
    takeProcessInput(processes, &numProcesses, n);

    // run algorithms one by one with pause
    printf("\nPress Enter to compare all algorithms...");
    getchar(); // consume leftover newline from scanf
    getchar(); // wait for user

    int chunkSizes[] = {n / 2, n/4, n/8, n/16, n/32, n/64, n/128, n/256, n/256};
    compareAlgorithms(processes, numProcesses, chunkSizes, n);

    // cleanup
    freeUpInitialHeap(basePointer);
    free(isUsed);
    free(basePointersArray);
    free(endPointersArray);
    
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
    // printing the chunk intiial and final addresses
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
void takeProcessInput(Process *processes, int *numProcesses, int heapSize)
{
    printf("Enter number of processes: ");
    scanf("%d", numProcesses);

    for(int i = 0; i < *numProcesses; i++)
    {
        sprintf(processes[i].name, "p%d", i);
        printf("Enter size for %s: ", processes[i].name);
        scanf("%d", &processes[i].size);

        if(processes[i].size > heapSize / 2)
        {
            printf("%s too large to fit in memory (greater than half of heap size)\n", processes[i].name);
            processes[i].allocatedChunk = -2; // mark the process as too large  
        }
        else    
        {
            processes[i].allocatedChunk = -1; // not yet allocated
        }
    }
}
void resetIsUsed()
{
    for(int i = 0; i < NUMBER_OF_CHUNKS; i++)
        isUsed[i] = 0;
}
void firstFit(Process *processes, int numProcesses, int *chunkSizes)
{
    printf("\n\n\tFirst Fit Allocation\n");
    resetIsUsed();  // reset the entire heap to be unused 

    for(int i = 0; i < numProcesses; i++)
    {
        if(processes[i].allocatedChunk == -2) continue; // if the process is too large, then skip it

        int allocated = 0; // flag to detemrine if the process was allocated or no
        for(int j = 0; j < NUMBER_OF_CHUNKS; j++)
        {
            if(isUsed[j] == 0 && processes[i].size <= chunkSizes[j])
            {
                isUsed[j] = 1;
                processes[i].allocatedChunk = j;
                allocated = 1;
                printf("%s allocated to chunk %d (size %d)\n", processes[i].name, j, chunkSizes[j]);
                break;
            }
        }
        if(!allocated)
            printf("%s could not be allocated.\n", processes[i].name);
    }
}
void bestFit(Process *processes, int numProcesses, int *chunkSizes)
{
    printf("\n\n\tBest Fit Allocation\n");
    resetIsUsed();

    for(int i = 0; i < numProcesses; i++)
    {
        if(processes[i].allocatedChunk == -2) continue;

        int bestIndex = -1;
        int bestSize = INT_MAX;
        for(int j = 0; j < NUMBER_OF_CHUNKS; j++)   // loop to find the best chunk to allocate the process in
        {
            if(isUsed[j] == 0 && processes[i].size <= chunkSizes[j] && chunkSizes[j] < bestSize)
            {
                bestSize = chunkSizes[j];
                bestIndex = j;
            }
        }
        if(bestIndex != -1)
        {
            isUsed[bestIndex] = 1;
            processes[i].allocatedChunk = bestIndex;
            printf("%s allocated to chunk %d (size %d)\n", processes[i].name, bestIndex, chunkSizes[bestIndex]);
        }
        else
        {
            printf("%s could not be allocated.\n", processes[i].name);
        }
    }
}
void worstFit(Process *processes, int numProcesses, int *chunkSizes)
{
    printf("\n\n\tWorst Fit Allocation\n");
    resetIsUsed();

    for(int i = 0; i < numProcesses; i++)
    {
        if(processes[i].allocatedChunk == -2) continue;

        int worstIndex = -1;
        int worstSize = -1;
        for(int j = 0; j < NUMBER_OF_CHUNKS; j++)
        {
            if(isUsed[j] == 0 && processes[i].size <= chunkSizes[j] && chunkSizes[j] > worstSize)
            {
                worstSize = chunkSizes[j];
                worstIndex = j;
            }
        }
        if(worstIndex != -1)
        {
            isUsed[worstIndex] = 1;
            processes[i].allocatedChunk = worstIndex;
            printf("%s allocated to chunk %d (size %d)\n", processes[i].name, worstIndex, chunkSizes[worstIndex]);
        }
        else
        {
            printf("%s could not be allocated.\n", processes[i].name);
        }
    }
}
float calculateUtilization(Process *processes, int numProcesses, int heapSize)
{
    int totalUsed = 0;

    for(int i = 0; i < numProcesses; i++)
    {
        if(processes[i].allocatedChunk >= 0)
            totalUsed += processes[i].size;
    }

    float utilization = ((float) totalUsed / heapSize) * 100.0;
    printf("Memory Utilization: %.2f%%\n", utilization);
    return utilization;
}
void compareAlgorithms(Process *originalProcesses, int numProcesses, int *chunkSizes, int heapSize)
{
    int sizeOfProcesses = numProcesses * sizeof(Process);
    Process *processesCopy = malloc(sizeOfProcesses);
    float ffUtil, bfUtil, wfUtil;

    // first fit 
    memcpy(processesCopy, originalProcesses, sizeOfProcesses);
    firstFit(processesCopy, numProcesses, chunkSizes);
    ffUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

    // bst fit
    memcpy(processesCopy, originalProcesses, sizeOfProcesses);
    bestFit(processesCopy, numProcesses, chunkSizes);
    bfUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

    // worst fit
    memcpy(processesCopy, originalProcesses, sizeOfProcesses);
    worstFit(processesCopy, numProcesses, chunkSizes);
    wfUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

    printf("\nResults:\n");
    if(ffUtil >= bfUtil && ffUtil >= wfUtil)
        printf("Best algorithm: First Fit with (%.2f%% utilization)\n", ffUtil);
    else if(bfUtil >= ffUtil && bfUtil >= wfUtil)
        printf("Best algorithm: Best Fit with (%.2f%% utilization)\n", bfUtil);
    else
        printf("Best algorithm: Worst Fit with (%.2f%% utilization)\n", wfUtil);
}