#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
const int NUMBER_OF_CHUNKS = 9;
const int MAX_PROCESSES = 100;
int *isUsed;                // array of if each chunk is used, 
// 0 - unused, 1 - used and has internal fragmentation, 2 - completely used
char **basePointersArray;  // array of pointers that keeps track of the base pointer for each chunk
char **endPointersArray;   // array of pointers that keeps track of the end pointer for each chunk `
int *spaceLeftInChunks;

typedef struct {
    int id;
    int size;             
    int allocatedChunk;   // which chunk it was allocated to, set to -1 if not allocated)
} Process;
Process *processes;
int counte=0;
// 0,1 ,2,3
// free up 3
int count = 0;

void split_chunks(int, char*);
void freeInitialHeap(char*);
void processInput(Process *processes, int *processCount, int heapSize);
void resetIsUsed();
void firstFit(Process *process, int numProcesses, int *chunkSizes);
void bestFit(Process *processes, int numProcesses, int *chunkSizes);
void worstFit(Process *processes, int numProcesses, int *chunkSizes);
float calculateUtilization(Process *processes, int numProcesses, int heapSize);
void compareAlgorithms(Process *originalProcesses, int numProcesses, int *chunkSizes, int heapSize);
void useFirstFit(Process*, int*);
void useWorstFit(Process, int*);
void useBestFit(Process, int*);
void customFree(int);
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
    char *basePointer = malloc((n * sizeof(char)));
    

    basePointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(char*));
    endPointersArray = malloc(NUMBER_OF_CHUNKS * sizeof(char*));
    split_chunks(n, basePointer);

    processes = malloc(sizeof(Process) * MAX_PROCESSES);
    // process input
    // takeProcessInput(processes, &numProcesses, n);

    
    spaceLeftInChunks = malloc(NUMBER_OF_CHUNKS * sizeof(int));
    int chunkSizes[] = {n / 2, n/4, n/8, n/16, n/32, n/64, n/128, n/256, n/256};
    for(int i=0;i<NUMBER_OF_CHUNKS;i++)
        spaceLeftInChunks[i] = chunkSizes[i];

    // compareAlgorithms(processes, numProcesses, chunkSizes, n);

    // main loop
    int running = 1;
    while(running)
    {
        // choice for the user
        int choice;
        int processNumber;
        int processSize;
        printf("\n\n1 - allocate process\n");
        printf("2 - free up process\n");
        printf("3 - View Remaining Space \n");
        printf("4 - Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        
        //switch case
        switch (choice)
        {
            // for allocation of the process
            case 1:
                // process input
                printf("Enter size for process P%d: ", count);
                scanf("%d", &processSize);
                Process p;
                p.id = count;
                p.size = processSize;
                p.allocatedChunk = -1; // by default the process has no allocated chunk
                processes[count].id = p.id; 
                processes[count].size = p.size; 
                count += 1;
                printf("\nnew count: %d\n", count);
                if (p.size > n/2)
                {
                    printf("\nThis process is too large to be allocated\n");
                    break;
                }
                // allocate logic here, call the required functions
                useFirstFit(&p, chunkSizes);
                // useWorstFit(p, chunkSizes);
                // useBestFit(p, chunkSizes);
                printf("\n--------PRINTING THE SPACE LEFT IN CHUNKS FOR DEBUG---------\n");
                for(int i=0;i<NUMBER_OF_CHUNKS;i++)
                {
                    printf("\nspaceleftinchunks[%d]: %d", i, spaceLeftInChunks[i]);
                }
                printf("\n\n--------SPACE LEFT IN CHUNKS DONE---------\n\n");
                break;
            // for freeing up the process
            case 2:
                printf("process number that is to be freed: ");
                scanf("%d", &processNumber);
                customFree(processNumber);
                break;
            case 3:
                printf("\n--------PRINTING THE SPACE LEFT IN CHUNKS FOR DEBUG---------\n");
                for(int i=0;i<NUMBER_OF_CHUNKS;i++)
                {
                    printf("\nspaceleftinchunks[%d]: %d", i, spaceLeftInChunks[i]);
                }
                printf("\n\n--------SPACE LEFT IN CHUNKS DONE---------\n\n");
                break;
            case 4:
                exit(0);
                running = 0;
                break;
            case 100:
                for(int i=0;i<5;i++)
                {
                    printf("\n\nid: %d", processes[i].id);
                    printf("\nallocated chunk: %d", processes[i].allocatedChunk);
                    printf("\nsize: %d", processes[i].size);
                    printf("\n\n");
                }
                break;
            default:
                printf("\nError! Invalid choice\n");
                break;
        }
    }
    // cleanup
    freeInitialHeap(basePointer);
    free(isUsed);
    free(basePointersArray);
    free(endPointersArray);
    
}
void split_chunks(int heapSize, char *basePointer)
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
    // PRINTING THE CHUNK BASE AND END POINTER LOOP COMMENT OUT IF DEBUGGING
    // printing the chunk intiial and final addresses
    // for(int i=0;i<NUMBER_OF_CHUNKS; i++)
    // {
        // printf("base pointer for %dth chunk: ", i);
        // printf("%p\n", basePointersArray[i]);
        // printf("end pointer for %dth chunk: ", i);
        // printf("%p\n", endPointersArray[i]);
        // printf("\n");
    // }
}
void freeInitialHeap(char *basePointer)
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
void resetIsUsed()
{
    for(int i = 0; i < NUMBER_OF_CHUNKS; i++)
        isUsed[i] = 0;
}
void firstFit(Process *process, int numProcesses, int *chunkSizes)
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
       if(spaceLeftInChunks[i]>=process->size && process->size < chunkSizes[i])
        {
            isUsed[i] = 1;
            spaceLeftInChunks[i] -= process->size;
            process->allocatedChunk = i;
            allocated = 1;
            printf("\nallocated to block %d\n", process->allocatedChunk);
            processes[count-1].allocatedChunk = process->allocatedChunk; 
            // printf("%s allocated to chunk %d (size %d)\n", processes[i]->name, i, chunkSizes[i]);
            break;
        } 
        // if the process requires the same amount of space as the entire chunk, set isused to 2
        else if(spaceLeftInChunks[i] >= process->size && process->size == chunkSizes[i])
        {
            isUsed[i] = 2;
            spaceLeftInChunks[i] -= process->size;
            process->allocatedChunk = i;
            printf("\nallocated to block %d\n", process->allocatedChunk);
            processes[count-1].allocatedChunk = process->allocatedChunk; 
            allocated = 1;
            break;
        }
    }
    if(!allocated)
    {
        printf("Process P%d could not be allocated, shortage of space ", process->id);
    }

    // for(int i = 0; i < numProcesses; i++)
    // {
    //     if(processes[i].allocatedChunk == -2) continue; // if the process is too large, then skip it

    //     int allocated = 0; // flag to detemrine if the process was allocated or no
    //     for(int j = 0; j < NUMBER_OF_CHUNKS; j++)
    //     {
    //         if(isUsed[j] == 0 && processes[i].size <= chunkSizes[j])
    //         {
    //             isUsed[j] = 1;
    //             processes[i].allocatedChunk = j;
    //             allocated = 1;
    //             printf("%s allocated to chunk %d (size %d)\n", processes[i].name, j, chunkSizes[j]);
    //             break;
    //         }
    //     }
    //     if(!allocated)
    //         printf("%s could not be allocated.\n", processes[i].name);
    // }
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
            printf("%s allocated to chunk %d (size %d)\n", processes[i].id, bestIndex, chunkSizes[bestIndex]);
        }
        else
        {
            printf("%s could not be allocated.\n", processes[i].id);
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
            printf("%s allocated to chunk %d (size %d)\n", processes[i].id, worstIndex, chunkSizes[worstIndex]);
        }
        else
        {
            printf("%s could not be allocated.\n", processes[i].id);
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


void useFirstFit(Process *process, int *chunkSizes)
{
    firstFit(process, 1, chunkSizes);
}
void useWorstFit(Process process, int *chunkSizes)
{
    // call function here
}
void useBestFit(Process process, int *chunkSizes)
{
    // call function here
}

void customFree(int id)
{
    Process dummy;

    dummy.allocatedChunk = processes[id].allocatedChunk;
    dummy.id = processes[id].id;
    dummy.size = processes[id].size;

    spaceLeftInChunks[dummy.allocatedChunk] += dummy.size;
    dummy.allocatedChunk = -1;
    dummy.id = -1;
    dummy.size = -1;
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

/*

enter proces number:0
enter process size:100

first
best
worst


txt file that has the heap 

3 files for 
first
best
worst

ansfirst
ansbest
answorst



*/