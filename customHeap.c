#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
const int NUMBER_OF_CHUNKS = 9;
const int MAX_PROCESSES = 100;

int **basePointersArray;  // array of pointers that keeps track of the base pointer for each chunk
int **endPointersArray;   // array of pointers that keeps track of the end pointer for each chunk 
/*
TODO
1. calculate internal, external and total fragmentation for each algorithm
2. write the process number(p0 would be 0) to the actual memory of the block it was assigned to
3. write the answers to text files  
4. make sure that freeing up rewrites the actual memory to be -1 or some other flag
5. make sure that freeing up also rewrites the text files to be correct
6. text files can be done last and if too complicated, just scrap it/do it in python/do it using ai
*/
// Process data type
typedef struct {
    int id;
    int size;
    int allocatedChunk;   // which chunk it was allocated to, set to -1 if not allocated)
    int firstChunk, bestChunk, worstChunk;
} Process;
Process *processes;

// heapState data type for each algorithm used
typedef struct{
    // array of if each chunk is used,  0 - unused, 1 - used and has internal fragmentation, 2 - completely used
    int *isUsed;
    int *spaceLeftInChunks;
    int internalFragment, externalFragment, totalFragment;
} heapState;
heapState firstFitHeap, bestFitHeap, worstFitHeap;

// the Process Name
int count = -1;

void split_chunks(int, int*);  // splits the initial heap into chunks
void freeInitialHeap(int*);    // frees up the initial allocated heap
void processInput(Process *processes, int *processCount, int heapSize); // not used function delete if still not used at the end
void resetIsUsed(heapState*); // resets the isused array for each heapstate
float calculateUtilization(Process *processes, int numProcesses, int heapSize); // rewrite this function
void compareAlgorithms(Process *originalProcesses, int numProcesses, int *chunkSizes, int heapSize);    // rewrite this function
void firstFit(Process process, int *chunkSizes, heapState*); // accepts the process and the chunksizes
void bestFit(Process process, int *chunkSizes, heapState*); // accepts the process and the chunksizes
void worstFit(Process process, int *chunkSizes, heapState*);    // accepts the process and the chunksizes
void useFirstFit(Process, int*, heapState*); // accepts the process, chunksizes, and its heapstate*
void useBestFit(Process, int*, heapState*);  // accepts the process, chunksizes, and its heapstate*
void useWorstFit(Process, int*, heapState*); // accepts the process, chunksizes, and its heapstate
void customFree(int, heapState*, int, int*);   // accepts the id of the process to be deleted, heapstate it belongs to, algorithm (0: firstfit, 1: bestfit, 2:worstfit)
void initializeHeapStates(heapState*, int*);    // isused and spaceleftinchunks array initialised for each heapstate
void displayChunks(heapState);
void calculateFragmentation(heapState*, int*, Process);
void displayFragment(heapState);
void assignData(Process p, int**, int**);
void writeDataToFile(heapState, char*, Process*, int**, int**);
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
    // isUsed = malloc(NUMBER_OF_CHUNKS * sizeof(int));
    // for(int i=0;i<NUMBER_OF_CHUNKS;i++){isUsed[i] = 0;}

    // allocating the initial heap
    int *basePointer = malloc((n * sizeof(int)));
    

    basePointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(int*));
    endPointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(int*));
    split_chunks(n, basePointer);
    Process p;
    processes = malloc(sizeof(Process) * MAX_PROCESSES);
    // process input
    // takeProcessInput(processes, &numProcesses, n);

    
    
    int chunkSizes[] = {n / 2, n/4, n/8, n/16, n/32, n/64, n/128, n/256, n/256};
    

    // compareAlgorithms(processes, numProcesses, chunkSizes, n);

    initializeHeapStates(&firstFitHeap, chunkSizes);
    initializeHeapStates(&bestFitHeap, chunkSizes);
    initializeHeapStates(&worstFitHeap, chunkSizes);
    // main loop
    int running = 1;
    while(running)
    {
        // choice for the user
        int choice;
        int processNumber;
        int processSize;
        printf("\n\n0 - Exit\n");
        printf("1 - allocate process\n");
        printf("2 - free up process\n");
        printf("3 - View Process Details \n");
        printf("4 - View Remaining Space \n\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        //switch case
        switch (choice)
        {
            case 0:
            {
                exit(0);
                running = 0;
                break;
            }
            // for allocation of the process
            case 1:
                // process input
                printf("Enter size for process P%d: ", (count+1));
                scanf("%d", &processSize);
                if(processSize<=0)
                {
                    printf("\nPlease Enter valid Process size\n");
                    break;
                }
                Process p;
                count += 1;
                p.id = count;
                p.size = processSize;
                p.allocatedChunk = -1; // by default the process has no allocated chunk
                processes[count].id = p.id; 
                processes[count].size = p.size; 
                // printf("\nnew count: %d\n", count);
                if (p.size > n/2)
                {
                    printf("\nThis process is too large to be allocated\n");
                    break;
                }
                // allocate logic here, call the required functions
                useFirstFit(p, chunkSizes, &firstFitHeap);
                useBestFit(p, chunkSizes, &bestFitHeap);
                useWorstFit(p, chunkSizes, &worstFitHeap);
                break;
            // for freeing up the process
            case 2:
            {
                printf("process number that is to be freed: ");
                scanf("%d", &processNumber);
                customFree(processNumber, &firstFitHeap, 0, chunkSizes);
                customFree(processNumber, &bestFitHeap, 1, chunkSizes);
                customFree(processNumber, &worstFitHeap, 2, chunkSizes);

                break;
            }
            // process details
            case 3:
            {
                printf("\nProcess Details:\n");
                for(int i=0;i<=count;i++)
                {
                    if(processes[i].id == -1)
                    {
                        printf("Process P%d was freed up\n", i);
                        printf("It occupied a space of: %d\n\n", processes[i].size);
                        continue;
                    }
                    printf("Process Name: P%d\n", processes[i].id);
                    printf("Process Size: %d\n", processes[i].size);
                    printf("Allocated Chunks of this process\n");
                    printf("Best Chunk: %d\n", processes[i].bestChunk);
                    printf("First Chunk: %d\n", processes[i].firstChunk);
                    printf("Worst Chunk: %d\n\n", processes[i].worstChunk);
                }
                break;
            }
            // space left in chunks + fragmentation
            case 4:
            {
                calculateFragmentation(&firstFitHeap, chunkSizes, processes);
                calculateFragmentation(&bestFitHeap, chunkSizes, processes);
                calculateFragmentation(&worstFitHeap, chunkSizes, processes);
                printf("\n\nFirst fit heap");
                displayChunks(firstFitHeap);
                printf("\n\nbest fit heap");
                displayChunks(bestFitHeap);
                printf("\n\nworst fit heap");
                displayChunks(worstFitHeap);
                break;
            }
            case 10:
                split_chunks(n, basePointer);
                break;
            case 100:
            {
                for(int i=0;i<=count;i++)
                {
                    printf("\n\nid: %d", processes[i].id);
                    printf("\nAllocated Chunks of this process\n");
                    printf("Best Chunk: %d\n", processes[i].bestChunk);
                    printf("First Chunk: %d\n", processes[i].firstChunk);
                    printf("Worst Chunk: %d\n", processes[i].worstChunk);
                    printf("\nsize: %d", processes[i].size);
                    printf("\n\n");
                }
                break;
            }
            case 101:
            {
                displayFragment(firstFitHeap);
                break;
            }
            default:
            {
                printf("\nError! Invalid choice\n");
                break;
            }
        }
    }
    // cleanup
    freeInitialHeap(basePointer);
    // free(isUsed);
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
    // printf("chunk 1 end pointer: %p\n", endPointersArray[0]);

    // assigning chunks 2-9 addresses
    for(int i=1;i<NUMBER_OF_CHUNKS;i++)
    {
        basePointersArray[i] = endPointersArray[i-1] + 1;
        endPointersArray[i] = basePointersArray[i] + (sizes[i] - 1);
    }
}
void freeInitialHeap(int *basePointer)
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
        // sprintf(processes[i].name, "p%d", i);
        // printf("Enter size for %s: ", processes[i].name);
        scanf("%d", &processes[i].size);

        if(processes[i].size > heapSize / 2)
        {
            // printf("%s too large to fit in memory (greater than half of heap size)\n", processes[i].name);
            processes[i].allocatedChunk = -2; // mark the process as too large  
        }
        else    
        {
            processes[i].allocatedChunk = -1; // not yet allocated
        }
    }
}
void resetIsUsed(heapState *current)
{
    for(int i = 0; i < NUMBER_OF_CHUNKS; i++)
        current->isUsed[i] = 0;
}
void firstFit(Process process, int *chunkSizes, heapState *current)
{
    // dont need to check for process larger than chunk size as its being handled above
    int allocated = 0;      // flag to detemrine if the process was allocated or no
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        // spaceLeftInChunks[0] = spaceLeftInChunks[0] - p0.space
        // if spcaeleftinchunks[i]>p.size
        // p.chunk = i
        // spaceLeftInChunks[i]-=p.size
        // if the process requires space less than the entire chunk, set isused to 1
       if(current->spaceLeftInChunks[i]>=process.size && process.size < chunkSizes[i])
        {
            current->isUsed[i] = 1;
            current->spaceLeftInChunks[i] -= process.size;
            process.firstChunk = i;
            allocated = 1;
            printf("\nFirst fit algorithm allocates this process to block %d\n", process.firstChunk);
            
            processes[count].firstChunk = process.firstChunk; 
            // printf("%s allocated to chunk %d (size %d)\n", processes[i].name, i, chunkSizes[i]);
            break;
        } 
        // if the process requires the same amount of space as the entire chunk, set isused to 2
        else if(current->spaceLeftInChunks[i] >= process.size && process.size == chunkSizes[i])
        {
            current->isUsed[i] = 2;
            current->spaceLeftInChunks[i] -= process.size;
            process.firstChunk = i;
            printf("\nallocated to block %d\n", process.firstChunk);
            processes[count].firstChunk = process.firstChunk; 
            allocated = 1;
            break;
        }
    }
    if(!allocated)
    {
        printf("\nProcess P%d could not be allocated, shortage of space\n", process.id);
    }
}
void calculateFragmentation(heapState *current, int *chunks, Process process)
{
    current->internalFragment = 0;
    current->externalFragment = 0;
    current->totalFragment = 0;
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        // current->spaceLeftInChunks[i];       spaceleftinchunks is the internal fragmentation???
        if(current->isUsed[i] == 1)
        {
            current->internalFragment += chunks[i] - process.size;
            // current->internalFragment += current->spaceLeftInChunks[i];
        }
        else if(current->isUsed[i] == 0)
        {
            current->externalFragment += chunks[i];
        }
    }
    current->totalFragment = current->internalFragment + current->externalFragment;
    // printf("Internal Fragmention: %d \t External Fragmention: %d \t Total Fragmentation: %d\n", current->internalFragment, current->externalFragment, current->totalFragment);
}
void bestFit(Process process, int *chunkSizes, heapState *current)
{
    int bestIndex = -1;
    int bestSize = INT_MAX;
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        if(process.size <= current->spaceLeftInChunks[i] && process.size <= chunkSizes[i] && chunkSizes[i] < bestSize)
        {
            bestSize = current->spaceLeftInChunks[i];
            bestIndex = i;
        }
    }
    if(bestIndex != -1)
    {
        // completely utilised chunk then set isused to 2 to indicate completely used
        if(current->spaceLeftInChunks[bestIndex] == process.size) 
        {
            current->isUsed[bestIndex] = 2; 
        }
        // if process size is less than the left chunk space, set isused to 1 to indicate partially used 
        else if(process.size < current->spaceLeftInChunks[bestIndex])
        {
            current->isUsed[bestIndex] = 1;
        }
        process.bestChunk = bestIndex;
        current->spaceLeftInChunks[bestIndex] -= process.size;
        processes[count].bestChunk = process.bestChunk; 
        printf("\nBest fit algorithm allocates this process to block %d\n", process.bestChunk);
    }
    else
    {
        printf("\nProcess P%d could not be allocated, shortage of space\n", process.id);
    }
}
void worstFit(Process process, int *chunkSizes, heapState *current)
{
    int worstIndex = -1;
    int worstSize = -1;
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        if(process.size <= current->spaceLeftInChunks[i] && process.size <= chunkSizes[i] && chunkSizes[i] > worstSize)
        {
            worstSize = current->spaceLeftInChunks[i];
            worstIndex = i;
        }
    }
    if(worstIndex != -1)
    {
        // completely utilised chunk then set isused to 2 to indicate completely used
        if(current->spaceLeftInChunks[worstIndex] == process.size) 
        {
            current->isUsed[worstIndex] = 2; 
        }
        // if process size is less than the left chunk space, set isused to 1 to indicate partially used 
        else if(process.size < current->spaceLeftInChunks[worstIndex])
        {
            current->isUsed[worstIndex] = 1;
        }
        process.worstChunk = worstIndex;
        current->spaceLeftInChunks[worstIndex] -= process.size;
        processes[count].worstChunk = process.worstChunk; 
        printf("\nWorst fit algorithm allocates this process to block %d\n", process.worstChunk);
    }
    else
    {
        printf("\nProcess P%d could not be allocated, shortage of space\n", process.id);
    }
    // for(int i = 0; i < numProcesses; i++)
    // {
    //     if(processes[i].allocatedChunk == -2) continue;

    //     int worstIndex = -1;
    //     int worstSize = -1;
    //     for(int j = 0; j < NUMBER_OF_CHUNKS; j++)
    //     {
    //         if(isUsed[j] == 0 && processes[i].size <= chunkSizes[j] && chunkSizes[j] > worstSize)
    //         {
    //             worstSize = chunkSizes[j];
    //             worstIndex = j;
    //         }
    //     }
    //     if(worstIndex != -1)
    //     {
    //         isUsed[worstIndex] = 1;
    //         processes[i].allocatedChunk = worstIndex;
    //         printf("%s allocated to chunk %d (size %d)\n", processes[i].id, worstIndex, chunkSizes[worstIndex]);
    //     }
    //     else
    //     {
    //         printf("%s could not be allocated.\n", processes[i].id);
    //     }
    // }
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
void useFirstFit(Process process, int *chunkSizes, heapState *firstFitHeap)
{
    firstFit(process, chunkSizes, firstFitHeap);
}
void useBestFit(Process process, int *chunkSizes, heapState *bestFitHeap)
{
    bestFit(process, chunkSizes, bestFitHeap);
}
void useWorstFit(Process process, int *chunkSizes, heapState *worstFitHeap)
{
    worstFit(process, chunkSizes, worstFitHeap);
}
void customFree(int id, heapState *current, int algorithm, int *chunkSizes)  // 0: firstfit, 1: bestfit, 2:worstfit
{
    Process *dummy;
    dummy = &processes[id];
    if(algorithm == 0)  // first fit
    {
        current->spaceLeftInChunks[dummy->firstChunk] += dummy->size;
        // if spaceleft + dummy process size is equal to total space of that chunk, it means that the entire chunk is unused so set isused to 0
        if((current->spaceLeftInChunks[dummy->firstChunk] + dummy->size) == chunkSizes[dummy->firstChunk])
        {
            current->isUsed[dummy->firstChunk] = 0;
        }
        else if((current->spaceLeftInChunks[dummy->firstChunk] + dummy->size) < chunkSizes[dummy->firstChunk])
        {
            current->isUsed[dummy->firstChunk] = 1;
        }
    }
    else if(algorithm == 1) // best fit
    {
        current->spaceLeftInChunks[dummy->bestChunk] += dummy->size;
        if((current->spaceLeftInChunks[dummy->bestChunk] + dummy->size) == chunkSizes[dummy->bestChunk])
        {
            current->isUsed[dummy->bestChunk] = 0;
        }
        else if((current->spaceLeftInChunks[dummy->bestChunk] + dummy->size) < chunkSizes[dummy->bestChunk])
        {
            current->isUsed[dummy->bestChunk] = 1;
        }
    }
    else if(algorithm == 2) // worst fit
    {
        current->spaceLeftInChunks[dummy->worstChunk] += dummy->size;
        if((current->spaceLeftInChunks[dummy->worstChunk] + dummy->size) == chunkSizes[dummy->worstChunk])
        {
            current->isUsed[dummy->worstChunk] = 0;
        }
        else if((current->spaceLeftInChunks[dummy->worstChunk] + dummy->size) < chunkSizes[dummy->worstChunk])
        {
            current->isUsed[dummy->worstChunk] = 1;
        }
    }
    dummy->allocatedChunk = -1;
    dummy->id = -1;
}
void initializeHeapStates(heapState *state, int *chunkSizes)
{
    state->isUsed = malloc(NUMBER_OF_CHUNKS * sizeof(int));
    state->spaceLeftInChunks = malloc(NUMBER_OF_CHUNKS * sizeof(int));
    state->externalFragment = 0;
    state->internalFragment = 0;
    state->totalFragment = 0;
    for(int i=0;i<NUMBER_OF_CHUNKS; i++)
    {
        state->spaceLeftInChunks[i] = chunkSizes[i];
        state->isUsed[i] = 0;
    }
}
void displayChunks(heapState current)
{   
    printf("\nSpace Remaining in the Chunks Details: \n");
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        printf("\nspaceleftinchunks[%d]: %d", i, current.spaceLeftInChunks[i]);
    }
    printf("\n");
    displayFragment(current);
}
void displayFragment(heapState current)
{
    printf("\nFragmentation Details:\n");
    printf("internal fragment: %d\n", current.internalFragment);
    printf("external fragment: %d\n", current.externalFragment);
    printf("total fragment: %d\n", current.totalFragment);
    printf("\n--------------------------------------\n");
}
void assignData(Process p, int **basePointers, int **endPointers)
{
    
}
void writeDataToFile(heapState state, char *filename, Process *processes, int **basePointers, int **endPointers)
{
    FILE *fp = fopen(filename, "w");
    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    // for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    // {
    //     int *start = basePointers[i];
    //     int *end = endPointers[i];
    //     for(int *address = start;address<=end;address++)
    //     {
    //         int data = -1;

    //         for(int p = 0; p <= count; p++)
    //         {
    //             if(processes[p].firstChunk == i || processes[p].bestChunk == i || processes[p].worstChunk == i)
    //             {
    //                 data = processes[i].id;
    //             }
    //         }
    //     }
    // }
}
// TODO: REWRITE THIS COMPARISON FUNCTION
// void compareAlgorithms(Process *originalProcesses, int numProcesses, int *chunkSizes, int heapSize)
// {
//     int sizeOfProcesses = numProcesses * sizeof(Process);
//     Process *processesCopy = malloc(sizeOfProcesses);
//     float ffUtil, bfUtil, wfUtil;

//     // first fit 
//     memcpy(processesCopy, originalProcesses, sizeOfProcesses);
//     firstFit(processesCopy, numProcesses, chunkSizes);
//     ffUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

//     // bst fit
//     memcpy(processesCopy, originalProcesses, sizeOfProcesses);
//     bestFit(processesCopy, numProcesses, chunkSizes);
//     bfUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

//     // worst fit
//     memcpy(processesCopy, originalProcesses, sizeOfProcesses);
//     worstFit(processesCopy, numProcesses, chunkSizes);
//     wfUtil = calculateUtilization(processesCopy, numProcesses, heapSize);

//     printf("\nResults:\n");
//     if(ffUtil >= bfUtil && ffUtil >= wfUtil)
//         printf("Best algorithm: First Fit with (%.2f%% utilization)\n", ffUtil);
//     else if(bfUtil >= ffUtil && bfUtil >= wfUtil)
//         printf("Best algorithm: Best Fit with (%.2f%% utilization)\n", bfUtil);
//     else
//         printf("Best algorithm: Worst Fit with (%.2f%% utilization)\n", wfUtil);
// }
