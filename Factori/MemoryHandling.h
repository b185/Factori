/* MemoryHandling.h
--------------------------------------------------------------
	Module Description - header module for MemoryHandling.c
--------------------------------------------------------------
*/


#pragma once
#ifndef __MEMORY_HANDLING_H__
#define __MEMORY_HANDLING_H__


// Library includes --------------------------------------------
#include <Windows.h>


// Projects includes -------------------------------------------
#include "HardCodedData.h"
#include "Queue.h"
#include "Lock.h"


//Functions Declarations
/// <summary>
///	 Description - This function receives a pointer to a Handle and attempts to close it and frees the allocated memory. 
///		If it fails to close it, it prints a relvant message.
/// </summary>
/// <param name="HANDLE* p_h_handle - A pointer to a WINAPI Handle to some sychronous object or file"></param>
void closeHandleProcedure(HANDLE* p_h_handle);
/// <summary>
///	 Description - This function receives a pointer to a threads' Handles array, a DWORD array of the threads IDs and a number of threads, and
///		 attempts to close all the opened handles to these threads and frees the allocated memory of the Handles array and the IDs. 
/// </summary>
/// <param name="HANDLE* p_h_threadsHandlesArray - A pointer to a threads Handles array"></param>
/// <param name="LPDWORD p_threadIds - A pointer to DWORD array where every cell represents some thread ID"></param>
/// <param name="int numberOfThreads - An integer representing the number of threads used"></param>
void closeThreadsProcedure(HANDLE* p_h_threadsHandlesArray, LPDWORD p_threadIds, int numberOfThreads);



/// <summary>
/// Description - This function receives a "file" struct and frees the "line" struct nested list within it, and then it frees the "file" struct itself.
/// </summary>
/// <param name="file* p_file - A pointer to a 'file' datatype (struct) that was used to frame some file's lines with byte-offsets"></param>
void freeTheFile(file* p_file);


/// <summary>
/// Description - This function receives a "primeFactors" struct and frees the "factor" struct nested list within it, and then it frees the "primeFactors" struct itself.
/// </summary>
/// <param name="primeFactors* p_numberPrimeFactors - A pointer to a 'primeFactors' datatype (struct) that was used to store the prime factors of a task"></param>
void freeThePrimeFactors(primeFactors* p_numberPrimeFactors);
/// <summary>
/// Description - This function receives a "factorizedString" struct and frees the string it points at, and then it frees the "factorizedString" struct itself.
/// </summary>
/// <param name="factorizedString* p_taskStringStruct - A pointer to a 'factorizedString' struct"></param>
void freeTheString(factorizedString* p_taskStringStruct);


/// <summary>
/// Description - This function receives a "threadPackage" struct and frees all of the data it points at e.g. Tasks Priorities queue struct,
///		Tasks file lock, queue's lock etc. and then it frees the "threadPackage" struct itself.
/// </summary>
/// <param name="threadPackage* p_threadParameters - A pointer to a 'threadPackage' struct that was used to hold all the parameters for the threads"></param>
void freeTheThreadPackage(threadPackage* p_threadParameters);
#endif //__MEMORY_HANDLING_H__