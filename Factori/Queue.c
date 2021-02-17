/* Queue.c
------------------------------------------------------------------------------------------
	Module Description - This module contains functions meant for handling a Queue struct
------------------------------------------------------------------------------------------
*/

// Library includes ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <assert.h>


// Projects includes ---------------------------------------------------------------------
#include "Queue.h"

// Constants -----------------------------------------------------------------------------
static const BOOL  STATUS_CODE_FAILURE = FALSE;
static const BOOL  STATUS_CODE_SUCCESS = TRUE;

static const DWORD SINGLE_OBJECT = 1;

static const BOOL QUEUE_IS_EMPTY = 2;
static const BOOL QUEUE_IS_NOT_EMPTY = 1;

//The next two constants are assuming there would be at least two values that the tasks'
// initials-position-byte-offsets won't be able to receive (no task will receive them either because
// tasks' values are upper bounded by 999,999,999). The reason for that is that
// the there has to be an empty line in the end of the Tasks list file, which means the last two
// characters whould have to be Carriage Return and Newline, so the Tasks Priorities list file,
// even if the Tasks list file is upper bounded by a size of 4GB, won't have an initial-position-byte-offset
// value greater than 4,294,967,294 which is ULONG_MAX-1, so ULONG_MAX-1 & ULONG_MAX are free for use as indicators.
static const DWORD TOP_SPECIAL_FAILURE_STATUS_CODE = ULONG_MAX;
static const DWORD TOP_QUEUE_WAS_EMPTY = ULONG_MAX-1;


static const BOOL POP_QUEUE_WAS_EMPTY = -2;



// Functions declerations ------------------------------------------------------------------
/// <summary>
///  Description - This function allocates memory (on the Heap) for a "cell" struct and returns a pointer to it, after
///		setting its' fields with the input cell value.
/// </summary>
/// <param name="DWORD value - An unsigned integer value"></param>
/// <returns>A pointer to an initiated 'cell' struct with an updated fields if successful, or NULL if failed.</returns>
static cell* cellStructMemoryAllocation(DWORD value);





//SEARCH CMD


// Functions definitions ------------------------------------------------------------------
queue* InitializeQueue()
{
	queue* p_queue = NULL;
	//Queue struct dynamic memory allocation
	if (NULL == (p_queue = (queue*)calloc(sizeof(queue), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a Queue struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Memory allocation for the Queue struct has been successful
	return p_queue;
}




DWORD Top(queue* p_queue)
{
	//Input integrity validation
	if (NULL == p_queue) {
		printf("Error: Bad input to function: %s\n", __func__);  return TOP_SPECIAL_FAILURE_STATUS_CODE;
	}
	//Returning the Top element's value in the queue
	if(QUEUE_IS_NOT_EMPTY == Empty(p_queue)) return (p_queue->firstCell)->value;
	else {
		printf("Thread no. %lu: Queue is already empty(TOP)!!\n\n", GetCurrentThreadId()); 
		return TOP_QUEUE_WAS_EMPTY; //Consider erasing the message to prevent overloading CMD
	}
}

BOOL Pop(queue* p_queue)
{
	cell* p_newFirstCell = NULL;
	//Input integrity validation
	if (NULL == p_queue) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}
	//Poping the top element in queue
	if (QUEUE_IS_NOT_EMPTY == Empty(p_queue)) {
		//Save the second-top element in the queue (might be NULL but its' okay)
		p_newFirstCell = (p_queue->firstCell)->p_next;
		//Free the top element in the queue
		free(p_queue->firstCell);
		//Define (connect) the queue's top element to be the second-top element
		p_queue->firstCell = p_newFirstCell;
		//if p_newFirstCell == NULL, then it sets the queue's state to QUEUE_IS_EMPTY according to Empty's definition
		return STATUS_CODE_SUCCESS;
	}
	else {//No need to validate Empty(p_queue) returns QUEUE_IS_EMPTY rather than STATUS_CODE_FAILURE,
		  // because the same input integrity validation was examined in the current function as well
		printf("Thread no. %lu: Queue is already empty(POP)!!\n\n", GetCurrentThreadId()); 
		return POP_QUEUE_WAS_EMPTY;			//Consider erasing the message to prevent overloading CMD
	}
}

BOOL Push(queue* p_queue, DWORD value)
{
	cell* p_currentCellInQueue = NULL;
	//Input integrity validation
	if ((NULL == p_queue) || (0 > value)) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}
	//Pushing another element to the end of the queue
	if (QUEUE_IS_NOT_EMPTY == Empty(p_queue)) {
		p_currentCellInQueue = p_queue->firstCell;
		//Search for last cell in the queue to append the new element to it
		while (NULL != p_currentCellInQueue->p_next) {
			//Continue searching for the last cell in the queue
			p_currentCellInQueue = p_currentCellInQueue->p_next;
		}
		if (NULL == (p_currentCellInQueue->p_next = cellStructMemoryAllocation(value))) {
			printf("Releasing the Queue & Exiting...\n");
			if (STATUS_CODE_SUCCESS == DestroyQueue(&p_queue))
				printf("Destroying queue succeeded...\n");
			else printf("Destroying queue failed...\n");
			return STATUS_CODE_FAILURE;
		}
	}
	else //No need to validate Empty(p_queue) returns QUEUE_IS_EMPTY rather than STATUS_CODE_FAILURE,
		 // because the same input integrity validation was examined in the current function as well
		if (NULL == (p_queue->firstCell = cellStructMemoryAllocation(value))) {
			printf("Releasing the Queue & Exiting...\n");
			if (STATUS_CODE_SUCCESS == DestroyQueue(&p_queue))
				printf("Destroying queue succeeded...\n");
			else printf("Destroying queue failed...\n");
			return STATUS_CODE_FAILURE;
		}
	//Inserting a new element to the end of the queue succeeded
	return STATUS_CODE_SUCCESS;
}




BOOL Empty(queue* p_queue)
{
	//Input integrity validation
	if (NULL == p_queue) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}
	//Return the emptiness state of the queue
	return (NULL == p_queue->firstCell) ? QUEUE_IS_EMPTY : QUEUE_IS_NOT_EMPTY;
}





BOOL DestroyQueue(queue** p_p_queue)
{
	queue* p_queue = NULL;
	cell* p_cell = NULL, *p_next = NULL;
	//Input integrity validation
	if (NULL == p_p_queue) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}
	//Receive the queue's address
	p_queue = *p_p_queue;

	printf("Beginning Queue destruction...\n");
	//Destroy Queue according to its' state: empty\not empty\queue address not legal
	switch (Empty(p_queue)) {
	case 1:  // Not Empty Queue
		p_cell = p_queue->firstCell;
		while (p_cell != NULL) {
			p_next = p_cell->p_next;
			free(p_cell);
			//Continue to next cell - it might be NULL which will end the loop & will set the queue's state to empty
			p_cell = p_next;
		}
		//Free queue
		free(p_queue);
		//Set the address pointed by the queue to NULL
		*p_p_queue = NULL;
		//Queue destruction succeeded
		return STATUS_CODE_SUCCESS;

	case 2:  // Empty Queue
		//Free queue
		free(p_queue);
		//Set the address pointed by the queue to NULL
		*p_p_queue = NULL;
		//Queue destruction succeeded
		return STATUS_CODE_SUCCESS;

	default:
		//Queue's address isn't legal...
		return STATUS_CODE_FAILURE;
	}
}


//......................................Static functions..........................................
static cell* cellStructMemoryAllocation(DWORD value)
{
	cell* p_cell = NULL;
	//Assert
	assert(0 <= value);
	//First cell (struct) in queue dynamic memory allocation
	if (NULL == (p_cell = (cell*)calloc(sizeof(cell), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a cell struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}
	//Updating cell with value
	p_cell->value = value;

	//Memory allocation for a cell struct has been successful
	return p_cell;
}