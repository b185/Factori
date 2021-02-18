/* Queue.h
---------------------------------------------------
	Module Description - header module for Queue.c
---------------------------------------------------
*/


#pragma once
#ifndef __QUEUE_H__
#define __QUEUE_H__


// Library includes --------------------------------------------
#include <Windows.h>


// Projects includes -------------------------------------------
#include "HardCodedData.h"



//Functions Declarations

/// <summary>
///	 Description - This function allocates memory (on the Heap) for a "queue" struct and returns a pointer to it (calloc)
/// </summary>
/// <returns>A pointer to an initiated 'queue' struct if successful, or NULL if failed.</returns>
queue* InitializeQueue();
/// <summary>
///  Description - This function implements the abstract utility to "take a look" at a queue's top value. The function returns
///		the top value of the nested-list implementing the queue. It also may return a status code if the queue is empty..
/// </summary>
/// <param name="queue* p_queue - A pointer to a queue struct"></param>
/// <returns>An unsigned integer of the value stored in the top 'cell' in the queue. Also in the case of an empty queue, the value ULONG_MAX-1 is returned, and in the case of failure, the value ULONG_MAX is returned (explanation is noted in near constants definitions)</returns>
DWORD Top(queue* p_queue);
/// <summary>
///  Description - This function implements the abstract utility to remove a queue's top value. The function removes the top cell of the nested-list
///		that implements the queue if the queue isn't empty, and does nothing if the queue is empty (besides printing a relevant message to the screen 
///		returning a relevant status code).
/// </summary>
/// <param name="queue* p_queue - A pointer to a queue struct"></param>
/// <returns>A BOOL value representing the function's outcome (Either pop removed the top cell or didn't). Popped the top cell (1), queue was empty (-2) or Failure (0)</returns>
BOOL Pop(queue* p_queue);
/// <summary>
///  Description - This function implements the abstract utility to append a cell to a queue. The function appends a cell to the end of the nested-list
///		containing the inputted value. Also the function might destroy the queue if any memory allocation error has occured.
/// </summary>
/// <param name="queue* p_queue - A pointer to a queue struct"></param>
/// <param name="DWORD value - An unsigned integer value to be inserted at the bottom of the queue"></param>
/// <returns>A BOOL value representing the function's outcome (The function managed to append additional cell to the end of the queue). Success (TRUE) or Failure (False)</returns>
BOOL Push(queue* p_queue, DWORD value);
/// <summary>
///  Description - This function implements the abstract utility to check a queue emptiness status. 
///		The function returns a status code
/// </summary>
/// <param name="queue* p_queue - A pointer to a queue struct"></param>
/// <returns>A BOOL value representing the function's outcome (Queue is empty or not). Empty (2), Not empty (1) or Failure (0)</returns>
BOOL Empty(queue* p_queue);
/// <summary>
///  Description - This function destroys the queue by releasing any memory allocated to the "queue" struct or its' "cell"s structs.
///		It also sets the queue's pointer to point at NULL address, and returns a final status code of the operation.
/// </summary>
/// <param name="queue** p_p_queue - A pointer to a pointer that points at a queue struct"></param>
/// <returns>A BOOL value representing the function's outcome (Queue mem. is released and its' pointer points at NULL). Success (TRUE) or Failure (False)</returns>
BOOL DestroyQueue(queue** p_p_queue);




#endif //__QUEUE_H__