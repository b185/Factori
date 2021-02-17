/* HardCodedData.h
---------------------------------------------------------------------
	Module Description - Header module that is meant to contain
		Macros, constants, structures' definitions etc.
		Without functions declerations. There is no HardCodedData.c!
---------------------------------------------------------------------
*/


#pragma once
#ifndef __HARD_CODED_DATA_H__
#define __HARD_CODED_DATA_H__


// Library includes -------------------------------------------------
#include <stdio.h>
#include <Windows.h>


// Constants
#define EMPTY_THREAD_PARAMETERS FALSE
#define MAX_LENGTH_TASK 10 //(999,999,999)


// Structures --------------------------------------------------------------------------------------------
	//line structure is used to store framed (single)line's data. The framing consists of the bytes-offset of the 
	//initial byte position of the line & number of bytes the line is comprised of until a Newline character.
	//The struct's purpose is to later read the data in the lines easily (since ReadFile uses bytes offsets)
typedef struct _line {
	//DWORD lineId;				 // The line ID which help the designated thread to locate the line and its' information 
	DWORD initialPositionByByte; // The byte ID of the first byte from which the line begins
	DWORD numberOfBytesInLine;	 // # of bytes in the line -> along with the previous field it is possible to calculate the final byte position
	struct _line *p_nextLine;	 // pointer to the next line struct
}line;

	//file structure is used to keep the entire file data as a nested-list of line structs
typedef struct _file { 
	DWORD numberOfLines;		 // # of null-terminated-strings (a file line) in the file
	DWORD numberOfBytes;		 // # of bytes throughout the entire file
	line* p_firstLineInFile;	 // pointer to the data of the first line in the file
}file;







	//factor structure is used to store a number's prime factor as a cell in a nested list. A nested-list is used
	// rather than an, for example, an array, since for every number it will be unknown at the beginning how many prime factors it is comprised of	
typedef struct _factor {
	DWORD factor;						// The value of the current prime factor cell
	struct _factor* p_nextFactor;		// pointer to the next factor struct
}factor;


	//primeFactors structure is used to keep a number's prime factors' nested list 
typedef struct _primeFactors {
	DWORD numberOfPrimeFactors;			// # of prime factors comprising the number
	DWORD numberOfCharacters;			// # of characters needed to represent all the prime factors' values
	factor* p_firstFactor;				// pointer to the next factors struct
}primeFactors;

	//factorizedString structure contains both the string of prime factors of a task & the string's size in bytes
typedef struct _factorizedString {
	DWORD stringSize;					// # of characters in the prime factors description string (null-terminated characters array)
	LPTSTR p_numberPrimeFactorsString;	// pointer to the string containing a number & its' prime factors components
}factorizedString;









//Queue struct - I chose to implement it by a nested list structure....
	//Cell struct - every element in the queue implemented by a nested-list
typedef struct _cell {
	DWORD value;						// Queue's cell's value - an unsigned datatype, since priorities are non-negative numbers
	struct _cell* p_next;				// pointer to the next element inside the Queue's nested-list
}cell;

	//Queue struct - main struct that hold the top element in the queue which is a cell datatype struct as defined above
typedef struct _queue {
	cell* firstCell;
	//An empty state would be when firstCell == NULL   instead of the field on the right//BOOL empty; 
}queue;



//Lock struct - I chose my lock to pocess 2 Mutexes & 1 Semaphore in order to solve the Readers\Writers problem with 
//				Starvation prevention
typedef struct _lock{
	HANDLE* p_h_turnstileMutex;										// Turstile Mutex as was learned in the recitation - Its' purpose is to choose between Writer & Readers
	HANDLE* p_h_readersCountingAtEntryOrDepartureProceduresMutex;	// Readers Mutex - Its' purpose is to create compartmentalization between Readers, while they may access the resource (allow counting Readers' number atomiclly)
	HANDLE* p_h_vaccantResourceStatusSemphore;						// EmptyRoom Semaphore - Its' purpose is as was taught in the recitation - to signal when the resource is vacant for accessing by either Readers or Writer
	int numberOfThreads;											// # Threads for Syncronous Objects signalling\unlocking timeouts calculations
	int numberOfTasks;												// # Tasks for Syncronous Objects signalling\unlocking timeouts calculations
}lock;



//Thread input parameters struct (package) - This is a struct main to combine all the inputs to a thread to assist
//											 it solving a set of tasks with syncronization to the other threads
typedef struct _threadPackage {
	//Resource 1
	char* p_tasksFilePath;					// pointer to the Tasks list file path (resource)
	lock* p_tasksFileLock;					// pointer to the Tasks list file resource lock
	//Resource 2
	queue* p_prioritiesQueue;				// pointer to the Tasks Priorities Queue struct (resource)
	lock* p_prioritiesQueueLock;			// pointer to the Tasks Priorities Queue struct resource lock
	//Resource 3 (This one will NEVER be accessed with Writers)
	file* p_tasksFileData;					// pointer to the Tasks file data struct (resource)
		//This thread doesn't need a lock because threads will only READ from it, which doesn't demand Synchronizing
}threadPackage;


#endif //__HARD_CODED_DATA_H__