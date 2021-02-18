/* MemoryHandling.c
------------------------------------------------------------------------------------------
	Module Description - This module contains functions meant for freeing dynamic memory 
	allocations & for closing WINAPI Handles.
------------------------------------------------------------------------------------------
*/

// Library includes ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes ---------------------------------------------------------------------
#include "MemoryHandling.h"


// Constants -----------------------------------------------------------------------------
static const BOOL FAILED_TO_CLOSE_HANDLE = FALSE;
static const BOOL STATUS_CODE_SUCCESS = TRUE;

// Functions declerations ------------------------------------------------------------------
/// <summary>
///  Description - This function receives a "line" struct which is a nested-list
///		and frees all elements in the list.
/// </summary>
/// <param name="line* p_topLine - A pointer to the head of the nested-list('line' struct)"></param>
static void freeTheLines(line* p_topLine);
/// <summary>
///  Description - This function receives a "factor" struct which is a nested-list
///		and frees all elements in the list.
/// </summary>
/// <param name="factor* p_smallestFactor - A pointer to the head of the nested-list('factor' struct)"></param>
static void freeTheFactors(factor* p_smallestFactor);




// Functions definitions ------------------------------------------------------------------

void closeHandleProcedure(HANDLE* p_h_handle)
{
	//Input integrity validation plus handle closer & pointer freeing operation
	if (NULL != p_h_handle) {
		//Attempting to close given handle
		if (FAILED_TO_CLOSE_HANDLE == CloseHandle(*p_h_handle)) {
			printf("Error: Failed close handle with code: %d.\n", GetLastError());
			printf("At file: %s\nAt line number: %d\nAt function: %s\n", __FILE__, __LINE__, __func__);
		}

		//Free the Handle pointer
		free(p_h_handle);
	}
}

void closeThreadsProcedure(HANDLE* p_h_threadsHandlesArray, LPDWORD p_threadIds, int numberOfThreads)
{
	int th = 0;
	//Input integrity validation
	if (0 >= numberOfThreads) { 
		printf("Error: Bad inputs to function: %s\n\n", __func__); return;
	}
	//Freeing Threads IDs DWORD array
	if (NULL != p_threadIds) free(p_threadIds);
	//Closing Handles
	if (NULL != p_h_threadsHandlesArray) {
		for (th; th < numberOfThreads; th++) {
			if (FAILED_TO_CLOSE_HANDLE == CloseHandle(*(p_h_threadsHandlesArray + th))) {
				printf("Error: Failed close handle with code: %d.\n", GetLastError());
				printf("At file: %s\nAt line number: %d\nAt function: %s\n\n", __FILE__, __LINE__, __func__);
			}
		}
		//Free threads Handles arrary
		free(p_h_threadsHandlesArray);
	}
}


//......................................file struct................................
void freeTheFile(file* p_file)
{
	//Freeing the Nested-List line structs in the file
	if (p_file->p_firstLineInFile != NULL)  freeTheLines(p_file->p_firstLineInFile);
	//Freeing the file struct
	if (p_file != NULL)  free(p_file);
	//For future use: It is possible to define file** p_p_file = &p_file, then, before free(p_file), place p_p_file=&p_file -> free -> *p_p_file= NULL
}

static void freeTheLines(line* p_topLine)
{
	line* p_currentLine = NULL;
	//Assert
	assert(p_topLine != NULL);
	//Freeing all the Nested-List of line-data cells
	while (p_topLine != NULL) {
		//Updating the current cell(line) to be the top line
		p_currentLine = p_topLine;
		//If the Nested-List still has line structures in it, the top pointer is updated to next line struct(cell)
		if (p_topLine->p_nextLine != NULL)  p_topLine = p_topLine->p_nextLine;
		else p_topLine = NULL;
		//Freeing the current top line struct(cell)
		free(p_currentLine);
	}
}




//......................................primeFactors struct................................
void freeThePrimeFactors(primeFactors* p_numberPrimeFactors)
{
	//Freeing the Nested-List factor structs in primeFactors
	if (p_numberPrimeFactors->p_firstFactor != NULL)  freeTheFactors(p_numberPrimeFactors->p_firstFactor);
	//Freeing the primeFactors struct
	if (p_numberPrimeFactors != NULL)  free(p_numberPrimeFactors);
	//For future use: It is possible to define file** p_p_file = &p_file, then, before free(p_file), place p_p_file=&p_file -> free -> *p_p_file= NULL
}

static void freeTheFactors(factor* p_smallestFactor)
{
	factor* p_currentFactor = NULL;
	//Assert
	assert(p_smallestFactor != NULL);
	//Freeing all the Nested-List of factor-data cells
	while (p_smallestFactor != NULL) {
		//Updating the current cell(factor) to be the smallest factor
		p_currentFactor = p_smallestFactor;
		//If the Nested-List still has factor structures in it, the smallest pointer is updated to next factor struct(cell)
		if (p_smallestFactor->p_nextFactor != NULL)  p_smallestFactor = p_smallestFactor->p_nextFactor;
		else p_smallestFactor = NULL;
		//Freeing the current smallest factor struct(cell)
		free(p_currentFactor);
	}
}


//......................................factorizedString struct................................

void freeTheString(factorizedString* p_taskStringStruct)
{
	//Freeing the dynamically allocated memory of the string
	if (NULL != p_taskStringStruct->p_numberPrimeFactorsString) free(p_taskStringStruct->p_numberPrimeFactorsString);
	//Freeing the task's prime factors string struct
	if (NULL != p_taskStringStruct) free(p_taskStringStruct);
}


//.......................................threadPackage struct....................................

void freeTheThreadPackage(threadPackage* p_threadParameters)
{
	//Destroying Tasks Priorities list Queue resource
	if (NULL != p_threadParameters->p_prioritiesQueue)
		if (STATUS_CODE_SUCCESS == DestroyQueue(&(p_threadParameters->p_prioritiesQueue)))
			printf("Tasks Priorities list Queue destruction succeeded!!!\n\n");
	//Destroying Tasks Priorities list Queue resource Lock
	if (NULL != p_threadParameters->p_prioritiesQueueLock)  
		if(STATUS_CODE_SUCCESS == DestroyLock(&(p_threadParameters->p_prioritiesQueueLock)))
			printf("Tasks Priorities list Queue resource Lock destruction succeeded!!!\n\n");
	//Destroying Tasks list file resource Lock
	if (NULL != p_threadParameters->p_tasksFileLock)		
		if(STATUS_CODE_SUCCESS == DestroyLock(&(p_threadParameters->p_tasksFileLock)))
			printf("Tasks list file resource Lock destruction succeeded!!!\n\n");
	//Destroying Tasks list file Data resource
	if (NULL != p_threadParameters->p_tasksFileData)		freeTheFile(p_threadParameters->p_tasksFileData);

	//Free the thread parameters struct (threadPackage)
	free(p_threadParameters);
}