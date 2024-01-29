/****************************************************************************************************************
CSL316 - Assignment 1 - Semester 6 (July-Nov) 2024
Programmer : Aditi Mukund Dhenge - BT21CSE077
Section : B
TA : 
Date due : January 24, 2024
Purpose : Simulating memory management as is done in interpreted languages like Python or Java.
          When a program requests requests a block of memory dynamically, the system must allocate memory, 
          and when a program terminates or frees memory previously requested, the system must deallocate memory. 

****************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to represent a memory block
typedef struct MemoryBlock {
    char* name;
    int start_address;
    int size;
    int ref_count;
    struct MemoryBlock* next;  
} MemoryBlock;

// Structure to represent a linked list of memory blocks - two lists : usedList and freeList
typedef struct MemoryList {
    MemoryBlock* head;
} MemoryList;

/***************************************************************
Function: initializeMemoryList

Use: Initializes a linked list representing memory blocks.

Arguments: 1.list - MemoryList pointer representing the linked list to be initialized.

Returns: void
**************************************************************/
void initializeMemoryList(MemoryList* list) {
    list->head = NULL;
}

/***************************************************************
Function: addMemoryBlock

Use: Adds a memory block to the linked list.

Arguments: 1.list - MemoryList pointer representing the linked list.
2.block - MemoryBlock pointer representing the memory block to be added.

Returns: void
*************************************************************/
void addMemoryBlock(MemoryList* list, MemoryBlock* block) {
    MemoryBlock* ptr = block;
    while(ptr->next){
        ptr = ptr->next;
    }
    ptr->next = list->head;
    list->head = block;
}

/***************************************************************
Function: removeMemoryBlock

Use: Removes a memory block from the linked list.

Arguments: 1.list - MemoryList pointer representing the linked list.
2.block - MemoryBlock pointer representing the memory block to be removed.

Returns: void
*************************************************************/
void removeMemoryBlock(MemoryList* list, MemoryBlock* block) {
    if (list->head == block) {
        list->head = block->next;
        block->next = NULL;
    } else {
        MemoryBlock* current = list->head;
        while (current != NULL && current->next != block) {
            current = current->next;
        }

        if (current != NULL) {
            current->next = block->next;
        }
    }
}

/***************************************************************
Function: compactMemory

Use: Performs memory compaction by merging adjacent free memory blocks.

Arguments: 1.freeList - MemoryList pointer representing the list of free memory blocks.
2.usedList - MemoryList pointer representing the list of used memory blocks.
3.memoryArray - MemoryBlock pointer representing the initial memory block.
4.blockName - char pointer representing the name of the memory block being compacted.

Returns: void
***********************************************************/
void compactMemory(MemoryList* freeList, MemoryList* usedList, MemoryBlock* memoryArray,char* blockName) {
    MemoryBlock* head1;
    MemoryBlock* prev1;
    MemoryBlock* prev = NULL;
    MemoryBlock* head = freeList->head;
    MemoryBlock* first_node = freeList->head;

    while (head != NULL) {
        head1 = head->next;
        prev1 = head;

        while (head1 != NULL) {
            if (head->start_address + head->size == head1->start_address || head1->start_address + head1->size == head->start_address) {
                break;
            }

            prev1 = head1;
            head1 = head1->next;
        }

        if (head1 != NULL) {
            prev1->next = head1->next;

            MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            char* allocatedName = (char*)malloc(strlen(blockName) + 1);
            strcpy(allocatedName, blockName);
            newBlock->name = allocatedName;
            newBlock->start_address = (head->start_address<head1->start_address) ?  head->start_address : head1->start_address;
            newBlock->size = head->size + head1->size;

            if (prev) {
                prev->next = head->next;
            } else {
                first_node = head->next;
            }

            newBlock->next = first_node;
            first_node = newBlock;
            freeList->head = first_node;

            head = first_node;
            continue;
        }

        prev = head;
        head = head->next;
    }
}

/***************************************************************
Function: allocateMemory

Use: Allocates memory of specified size from the free memory list.

Arguments: 1.freeList - MemoryList pointer representing the list of free memory blocks.
2.usedList - MemoryList pointer representing the list of used memory blocks.
3.size - integer representing the size of memory to be allocated.
4.memoryArray - MemoryBlock pointer representing the initial memory block.
5.blockName - char pointer representing the name of the memory block being allocated.

Returns: int - the start address of the allocated memory block or -1 if allocation fails.
***********************************************************/
int allocateMemory(MemoryList* freeList, MemoryList* usedList, int size, MemoryBlock* memoryArray,char* blockName) {

    MemoryBlock* current = freeList->head;
    MemoryBlock* prev = NULL;

    while (current != NULL) {
    
        if (current->size >= size) {

            // Allocate memory from the current free block
            int allocatedAddress = current->start_address;

            MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            
            char* allocatedName = (char*)malloc(strlen(blockName) + 1);
            strcpy(allocatedName, blockName);
            newBlock->name = allocatedName;

            newBlock->ref_count = 1;
            newBlock->size = size;
            newBlock->start_address = allocatedAddress;
            newBlock->next = NULL;
            current->start_address += size;
            current->size -= size;

            // Move the allocated block to the used list
            addMemoryBlock(usedList, newBlock);

            return allocatedAddress;
        }

        prev = current;
        current = current->next;
    }

    compactMemory(freeList, usedList, memoryArray,blockName);

    // Retry allocation after compaction
    current = freeList->head;
    prev = NULL;

    while (current != NULL) {

        if (current->size >= size) {

            // Allocate memory from the current free block
            int allocatedAddress = current->start_address;

            MemoryBlock* newBlock = (MemoryBlock*)malloc(sizeof(MemoryBlock));
            
            char* allocatedName = (char*)malloc(strlen(blockName) + 1);
            strcpy(allocatedName, blockName);
            newBlock->name = allocatedName;
            
            newBlock->ref_count = 1;
            newBlock->size = size;
            newBlock->start_address = allocatedAddress;
            newBlock->next = NULL;
            current->start_address += size;
            current->size -= size;
            addMemoryBlock(usedList, newBlock);

            return allocatedAddress;
        }

        prev = current;
        current = current->next;
    }

    printf("Error: Not enough free memory\n");
    return -1;
}

/***************************************************************
Function: assignMemory

Use: Assign memory to the destination block and increase its reference count.

Arguments: 1.usedList - MemoryList pointer representing the list of used memory blocks.
2.sourceName - char pointer representing the name of the source memory block.
3.destName - char pointer representing the name of the destination memory block.

Returns: void
************************************************************/
void assignMemory(MemoryList* usedList, char* sourceName, char* destName) {
    MemoryBlock* sourceBlock = usedList->head;
    MemoryBlock* destBlock = usedList->head;

    // Find the blocks with the specified names
    while (sourceBlock != NULL && strcmp(sourceBlock->name, sourceName) != 0) {
        sourceBlock = sourceBlock->next;
    }

    while (destBlock != NULL && strcmp(destBlock->name, destName) != 0) {
        destBlock = destBlock->next;
    }

    if (sourceBlock && destBlock) {
        destBlock->ref_count++;
        printf("Assigned memory from %s to %s\n", sourceName, destName);
    } else {
        printf("Error: Invalid assignment\n");
    }
}

/***************************************************************
Function: deallocateMemory

Use: Deallocates memory by moving the block to the free memory list when reference count becomes zero.

Arguments: 1.freeList - MemoryList pointer representing the list of free memory blocks.
2.usedList - MemoryList pointer representing the list of used memory blocks.
3.blockName - char pointer representing the name of the memory block to be deallocated.

Returns: void
************************************************************/
void deallocateMemory(MemoryList* freeList, MemoryList* usedList, char* blockName) {
    MemoryBlock* block = usedList->head;
    MemoryBlock* prev = NULL;

    // Find the block with the specified name
    while (block != NULL && strcmp(block->name, blockName) != 0) {
        block = block->next;
    }

    if (block) {
        block->ref_count--;

        // Deallocation only happens when the reference count goes to 0
        if (block->ref_count == 0) {
            // Move the block back to the free list
            removeMemoryBlock(usedList, block);
            addMemoryBlock(freeList, block);
            printf("Deallocated memory for %s\n", blockName);
            return;
        }
    }

    printf("Error: Invalid deallocation\n");
}

/***************************************************************
Function: printMemoryStatus

Use: Prints the status of used and free memory blocks to the output file.

Arguments: 1.freeList - MemoryList pointer representing the list of free memory blocks.
2.usedList - MemoryList pointer representing the list of used memory blocks.
3.output - FILE pointer representing the output file.

Returns: void
************************************************************/
void printMemoryStatus(MemoryList* freeList, MemoryList* usedList, FILE* output) {
    fprintf(output, "Used Blocks:\n");
    MemoryBlock* currentUsed = usedList->head;
    while (currentUsed != NULL && currentUsed->size != 0) {
        fprintf(output, "Name: %s, Address: %d, Size: %d, Ref Count: %d\n", currentUsed->name, currentUsed->start_address, currentUsed->size, currentUsed->ref_count);
        currentUsed = currentUsed->next;
    }

    fprintf(output, "\nFree Blocks:\n");
    MemoryBlock* currentFree = freeList->head;
    while (currentFree != NULL && currentFree->size != 0) {
        fprintf(output, "Name: %s, Address: %d, Size: %d\n", currentFree->name, currentFree->start_address, currentFree->size);
        currentFree = currentFree->next;
    }
}


/***************************************************************
Function: processTransactions

Use: Processes transactions from an input file, performing memory allocation, assignment, and deallocation.

Arguments: 1.inputFile - FILE pointer representing the input file containing transactions.
2.freeList - MemoryList pointer representing the list of free memory blocks.
3.usedList - MemoryList pointer representing the list of used memory blocks.
4.memoryArray - MemoryBlock pointer representing the initial memory block.

Returns: void
***********************************************************/
void processTransactions(FILE* inputFile, MemoryList* freeList, MemoryList* usedList, MemoryBlock* memoryArray) {
    char operation[20];
    char blockNameA[10];
    char blockNameB[10];
    int size;

    while (fscanf(inputFile, "%s", operation) != EOF) {
        if (strcmp(operation, "allocate") == 0) {
            fscanf(inputFile, "%s %d", blockNameA, &size);

            int allocatedAddress = allocateMemory(freeList, usedList, size, memoryArray,blockNameA);
            if (allocatedAddress != -1) {
                printf("Allocated memory for %s at address %d\n", blockNameA, allocatedAddress);
            } else {
                printf("Error: Not enough free memory for %s\n", blockNameA);
            }
        } else if (strcmp(operation, "assign") == 0) {
            fscanf(inputFile, "%s %s", blockNameA, blockNameB);
            assignMemory(usedList, blockNameA, blockNameB);
        } else if (strcmp(operation, "deallocate") == 0) {
            fscanf(inputFile, "%s", blockNameA);
            deallocateMemory(freeList, usedList, blockNameA);
        } else {
            printf("Error: Unknown operation\n");
        }
    }
}
int main() {
    // Create two linked lists to represent blocks in use and not in use
    MemoryList freeList;
    MemoryList usedList;
    initializeMemoryList(&freeList);
    initializeMemoryList(&usedList);

    // Create a single memory node of size 1000
    MemoryBlock memoryNode;
    memoryNode.name = "start";
    memoryNode.start_address = 0;
    memoryNode.size = 1000;
    memoryNode.ref_count = 0;
    memoryNode.next = NULL;

    // Add the initial block to the free list
    addMemoryBlock(&freeList, &memoryNode);

    FILE* inputFile = fopen("/Users/aditidhenge/Desktop/LP/input.txt", "r");
    if (inputFile == NULL) {
        fprintf(stderr, "Error: Unable to open input file\n");
        return 1;
    }

    FILE* outputFile = fopen("/Users/aditidhenge/Desktop/LP/output6.txt", "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: Unable to open output file\n");
        return 1;
    }

    processTransactions(inputFile, &freeList, &usedList, &memoryNode);

    fprintf(outputFile, "\nMemory Status After Transactions:\n");
    printMemoryStatus(&freeList, &usedList, outputFile);

    fclose(inputFile);

    fclose(outputFile);

    return 0;
}