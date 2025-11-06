#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include<windows.h>
const int NUMBER_OF_CHUNKS = 9;
const int MAX_PROCESSES = 100;

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
heapState firstFitHeap, bestFitHeap, worstFitHeap; // 9

typedef struct{
    char *address;
    int data;
}Memory;
Memory *ffMemoryBlock, *bfMemoryBlock, *wfMemoryBlock;
// the Process Name
int count = -1;

long long nano_time();
void freeInitialHeap(int*);    // frees up the initial allocated heap
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
void initializeMemoryBlocks(Memory**, int, int*);    // isused and spaceleftinchunks array initialised for each heapstate 
void displayChunks(heapState);
void calculateFragmentation(heapState*, int*);
void displayFragment(heapState);
void assignData(Memory*, int, int*);
void writeDataToFile(heapState, char*, Process*, Memory*, int );
void clearFile(char*, int, Memory*);

int main()
{
    int n = 1024;
    long long start, end;
    long long firstTime, bestTime, worstTime;
    // allocating the initial heap
    int *basePointer = malloc((n * sizeof(int)));

    Process p;
    processes = malloc(sizeof(Process) * MAX_PROCESSES);
    
    int chunkSizes[] = {n / 2, n/4, n/8, n/16, n/32, n/64, n/128, n/256, n/256};
        
    // heap state initialisation
    initializeHeapStates(&firstFitHeap, chunkSizes);
    initializeHeapStates(&bestFitHeap, chunkSizes);
    initializeHeapStates(&worstFitHeap, chunkSizes);
    // Memory Block initialisation
    initializeMemoryBlocks(&ffMemoryBlock, n, basePointer);
    initializeMemoryBlocks(&bfMemoryBlock, n, basePointer);
    initializeMemoryBlocks(&wfMemoryBlock, n, basePointer);
    // main loop
    int running = 1;
    while(running)
    {
        // choice for the user
        int choice, processNumber, processSize;
        printf("\n\n0 - Exit\n");
        printf("5 - Help \n");
        printf("1 - allocate a single process\n");
        printf("2 - free up process\n");
        printf("3 - View Process Details \n");
        printf("4 - View Remaining Space and fragmentation \n");
        printf("6 - Write the data to Text File \n");
        printf("7 - Reset Text File Data \n");
        printf("8 - Time Taken by the algorithms \n\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        switch (choice)
        {
            case 0:
            {
                // exit(0);
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
                p.firstChunk = -1;
                p.worstChunk = -1;
                p.bestChunk = -1;
                processes[count].id = p.id; 
                processes[count].size = p.size; 
                // printf("\nnew count: %d\n", count);
                if (p.size > n/2)
                {
                    printf("\nThis process is too large to be allocated\n");
                    break;
                }
                // allocate logic here, call the required functions
                start = nano_time();
                useFirstFit(p, chunkSizes, &firstFitHeap);
                end = nano_time();
                firstTime = end-start;

                start = nano_time();
                useBestFit(p, chunkSizes, &bestFitHeap);
                end = nano_time();
                bestTime = end-start;

                start = nano_time();
                useWorstFit(p, chunkSizes, &worstFitHeap);
                end = nano_time();
                worstTime = end - start;
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
                calculateFragmentation(&firstFitHeap, chunkSizes);
                calculateFragmentation(&bestFitHeap, chunkSizes);
                calculateFragmentation(&worstFitHeap, chunkSizes);
                printf("\n\nFirst fit heap");
                displayChunks(firstFitHeap);
                printf("\n\nbest fit heap");
                displayChunks(bestFitHeap);
                printf("\n\nworst fit heap");
                displayChunks(worstFitHeap);
                break;
            }
            case 5:
            {
                printf("\n Total Heap size: %d", n);
                printf("\n Number of chunks: %d", NUMBER_OF_CHUNKS);
                printf("\nChunks are numbered 0-8");
                for(int i=0;i<NUMBER_OF_CHUNKS;i++)
                {
                    printf("\nChunk %d Space: %d", i, chunkSizes[i]);
                }
                break;
            }
            case 6:
            {
                assignData(ffMemoryBlock, 0, chunkSizes);
                writeDataToFile(firstFitHeap, "first.txt", processes, ffMemoryBlock, n);

                assignData(bfMemoryBlock, 1, chunkSizes);
                writeDataToFile(bestFitHeap, "best.txt", processes, bfMemoryBlock, n);

                assignData(wfMemoryBlock, 2, chunkSizes);
                writeDataToFile(worstFitHeap, "worst.txt", processes, wfMemoryBlock, n);
                break;
            }
            case 7:
                clearFile("first.txt", n, ffMemoryBlock);
                clearFile("best.txt", n, bfMemoryBlock);
                clearFile("worst.txt", n, wfMemoryBlock);
                break;
            case 8:
            {
                printf("Time measured in nanoseconds\n");
                printf("first fit algorithm time: %d\n", firstTime);
                printf("best fit algorithm time: %d\n", bestTime);
                printf("worst fit algorithm time: %d\n", worstTime);
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

}

void freeInitialHeap(int *basePointer)
{
    free(basePointer);
    basePointer = NULL;
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
void calculateFragmentation(heapState *current, int *chunks)
{
    current->internalFragment = 0;
    current->externalFragment = 0;
    current->totalFragment = 0;
    int internal = 0, external = 0;
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
    {
        // current->spaceLeftInChunks[i];       spaceleftinchunks is the internal fragmentation???
        if(current->isUsed[i] == 1)
        {
            internal += current->spaceLeftInChunks[i];
        }
        else if(current->isUsed[i] == 0)
        {
            external += current->spaceLeftInChunks[i];
        }
    }
    current->internalFragment = internal;
    current->externalFragment = external;
    
    current->totalFragment = current->internalFragment + current->externalFragment;
    printf("Internal Fragmention: %d \t External Fragmention: %d \t Total Fragmentation: %d\n", current->internalFragment, current->externalFragment, current->totalFragment);
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
void assignData(Memory *current, int algorithm, int* chunkSizes)
{
    int heapTotal = 0;
    for (int t = 0; t < NUMBER_OF_CHUNKS; t++) heapTotal += chunkSizes[t];
    for (int i = 0; i < heapTotal; i++) current[i].data = -1;

    // off set that keeps track per chunk
    int chunkWriteOffset[9] = {0};

    for (int i = 0; i <= count; i++)
    {
        Process p = processes[i];
        int chunkIndex =
            (algorithm == 0) ? p.firstChunk :
            (algorithm == 1) ? p.bestChunk  :
            (algorithm == 2) ? p.worstChunk : -1;

        if (chunkIndex == -1 || p.id == -1) continue;

    
        int startIndex = 0;
        for (int k = 0; k < chunkIndex; k++) startIndex += chunkSizes[k];

        int offset = chunkWriteOffset[chunkIndex];


        if (offset + p.size > chunkSizes[chunkIndex]) 
        {
            printf("chunk %d overflow when placing P%d (size %d)\n",chunkIndex, p.id, p.size);
            continue;
        }


        for (int j = 0; j < p.size; j++)
            current[startIndex + offset + j].data = p.id;

        // update the chunk tracking offset
        chunkWriteOffset[chunkIndex] += p.size;
    }
}

void writeDataToFile(heapState state, char *filename, Process *processes, Memory *current, int size)
{
    FILE *fp = fopen(filename, "w");
    if(!fp)
    {
        printf("Error opening file %s\n", filename);
        return;
    }
    for(int i = 0;i<size;i++)
    {
        char *line = malloc(50);
        sprintf(line, "%s -> P%d\n" , current[i].address, current[i].data);
        fputs(line, fp);
        free(line);
    }
    fclose(fp);
}
void initializeMemoryBlocks(Memory **current, int size, int *basepointer)
{
    *current = malloc(size * sizeof(Memory));
    for(int i=0;i<size;i++)
    {
        (*current)[i].data = -1;
        (*current)[i].address = malloc(32);
        sprintf((*current)[i].address, "%p", (void*)(basepointer+i));
    }
}
void clearFile(char *filename, int size, Memory *current)
{
    FILE *fp = fopen(filename, "w");
    for(int i = 0;i<size;i++)
    {
        char *line = malloc(50);
        sprintf(line, "%s -> -1\n" , current[i].address, current[i].data);
        fputs(line, fp);
        free(line);
    }
    fclose(fp);
}
long long nano_time()
{
    LARGE_INTEGER freq, t;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    return (long long)( (1e9 * t.QuadPart) / freq.QuadPart );
}
