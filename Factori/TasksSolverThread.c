/* TasksSolverThread.c
----------------------------------------------------------------------------------------
	Module Description - This module contains the thread routine that will be executed
		by every thread created by the main thread (the process). It will also contain
		additional functions meant to assist the thread to locate tasks in the Tasks 
		list file (via byte-offsets positions), read some tasks' value from the
		same file and printing the thread's solutions ,to the tasks it solved, to the 
		output file. (The solving functions are located in a different module)
-----------------------------------------------------------------------------------------
*/

// Library includes ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes --------------------------------------------------------------------
#include "TasksSolverThread.h"



// Constants
static const BOOL  STATUS_CODE_FAILURE = FALSE;
static const BOOL  STATUS_CODE_SUCCESS = TRUE;

static const DWORD SINGLE_OBJECT = 1;



static const BOOL POP_QUEUE_WAS_EMPTY = (BOOL)-2;

static const DWORD TIMEOUT_BASE_CONSTANT_MS = 100; //100 milli-second

	//Write lock constants
static const BOOL WRITE_ACCESS_GRANTED = TRUE;
static const BOOL STATUS_FILE_WRITING_FAILED = (BOOL)0;
static const BOOL STATUS_NEW_END_OF_FILE_SET_FAILED = 0;

	//Read lock constant
static const BOOL READ_ACCESS_GRANTED = TRUE;
//static const BOOL STATUS_FILE_READING_FAILED = (BOOL)0; ---->>> in fetchMostPrioritizedTaskSizeInBytes(.) which is in SolveTasks.c module 





// Functions decleraitions ---------------------------------------------------------------
/// <summary>
///  Description - This function allocates (on the Heap - dynamic mem.) memory for a Handle and 
///		uses CreateFile WINAPI function to create a Handle to the Tasks list file (with Reading and writing permissions).
///		The file opening routine is also defined to include sharing for both reading and writing requests to enable
///		synchronizing between threads attempting to access the file, e.g. in the case of multiple Readers (This sharing permission
///		is needed even for the state in which we open multiple handles from different threads, not only when accessing the file simultenously).
///		It returns a pointer to newly created file Handle.
/// </summary>
/// <param name="char* p_filePath - A pointer to the commandline argument (string) that represents the Tasks list file's path (absolute\relative)"></param>
/// <returns>A pointer to a Handle to the Tasks file allocated on the heap if successful, or NULL if failed</returns>
static HANDLE* openFileForReadingAndWriting(char* p_filePath);
/// <summary>
///  Description - This function scans through the given Tasks-list file (Tasks.txt) data ("file") struct's lines ("line") for the initial byte offset (from the
///		beginning of the file) that matches the value stored in the input variable taskByteOffsetFromBeginningOfTheFile. The "line" struct 
///		that will have its' initialPositionByByte field's value identical to the input taskByteOffsetFromBeginningOfTheFile, will also hold
///		the number of bytes within the line that pocesses the task needed to be fetched (numberOfBytesInLine). The output is the number of bytes in 
///		that line (That holds the desired task) in the Tasks file.
/// </summary>
/// <param name="file* p_tasksFileData - A pointer to the Tasks-list file data struct assembled with the file's lines' byte-offsets (nested-list)"></param>
/// <param name="DWORD taskByteOffsetFromBeginningOfTheFile - An unsigned integer representing the initial byte-position of the most prioritized task currently being fetched"></param>
/// <returns>An unsigned integer representing the number of bytes in the line, in the Tasks file, that holds the task's value's string</returns>
static DWORD fetchMostPrioritizedTaskSizeInBytes(file* p_tasksFileData, DWORD taskByteOffsetFromBeginningOfTheFile);
/// <summary>
///	 Description - This function receives as input a pointer to the Handle of the Tasks-list (Tasks.txt) file, a string describing the decomposition
///		of a task's value into the prime factors it is comprised of, and the length of this string in bytes. The function relocates the file's Handle
///		to the end-of-the Tasks file, prints the string to that location and redefines the end-of-file byte location to the end of the printed string.  
/// </summary>
/// <param name="HANDLE* p_h_tasksListFile - A pointer to the Handle of the Tasks-list text file (Tasks.txt)"></param>
/// <param name="LPTSTR p_numberPrimeFactorsString - A pointer to a TCHAR string containing formal phrasing of a number and the prime factors it is comprised of, and terminates with '\0'"></param>
/// <param name="DWORD stringLength - An unsigned integer representing the size of the input task's prime factors string"></param>
/// <returns>A BOOL value representing the function's outcome (The current task's prime-factors-string was printed to the Tasks file and a new EOF was defined at the string's end). Success (TRUE) or Failure (False)</returns>
static BOOL printSolvedTaskToTasksListFile(HANDLE* p_h_tasksListFile, LPTSTR p_numberPrimeFactorsString, DWORD stringLength);

// Functions definitions ---------------------------------------------------------------

/*
BOOL WINAPI solveSingleTaskThread(LPVOID lpParam)
{
	HANDLE* p_h_taskList = NULL;
	factorizedString* p_taskPrimeFactorsStringData = NULL;
	DWORD params = 0;
	//Check whether lpParam is NULL - Input integrity validation
	if (NULL == lpParam) return EMPTY_THREAD_PARAMETERS;

	//Parameters input conversion from void pointer to section struct pointer by explicit type casting
	params = *((LPDWORD)lpParam);

	//Task given for solving is the number '0'
	if (params == 0) return STATUS_CODE_FAILURE;




	//Solve Task.....
	printf("Beginning solving task: %lu...\n", params);
	if (NULL == (p_taskPrimeFactorsStringData = receivePrimeFactorizedListString((int)params))) {
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}


	//Open a Handle to the Tasks list file - GLOBAL PATH
	if (INVALID_HANDLE_VALUE == *(p_h_taskList = openFileForReadingAndWriting(p_filePath))) {
		printf("Error: Failed to open tasks list file (input) with code: %d.\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		freeTheString(p_taskPrimeFactorsStringData);
		return STATUS_CODE_FAILURE;
	}

	//Write the solution (Task's number prime factors string) to the Tasks list file
	if (STATUS_CODE_FAILURE == printSolvedTaskToTasksListFile(p_h_taskList, p_taskPrimeFactorsStringData->p_numberPrimeFactorsString, p_taskPrimeFactorsStringData->stringSize)) {
		freeTheString(p_taskPrimeFactorsStringData);
		closeHandleProcedure(p_h_taskList);
		return STATUS_CODE_FAILURE;
	}

	//Free the task prime factors composition string reperesentation struct & close Tasks list file handle
	freeTheString(p_taskPrimeFactorsStringData);
	closeHandleProcedure(p_h_taskList);

	//Solving the task & writing the solution succeeded....
	return STATUS_CODE_SUCCESS;
}
*/


BOOL WINAPI tasksSolverThread(LPVOID lpParam)
{
	threadPackage* p_params = NULL;
	HANDLE* p_h_tasksList = NULL;
	DWORD taskNumberStringNumberOfBytes = 0, task = 0, taskInitialByteOffsetChosenByPriority = 0;
	factorizedString* p_taskPrimeFactorsStringData = NULL;
	//Check whether lpParam is NULL - Input integrity validation
	if (NULL == lpParam) return EMPTY_THREAD_PARAMETERS;

	//Parameters input conversion from void pointer to section struct pointer by explicit type casting
	p_params = (threadPackage*)lpParam;


	//Open a Handle to the Tasks list file - GLOBAL PATH
	if (NULL == (p_h_tasksList = openFileForReadingAndWriting(p_params->p_tasksFilePath))) {
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Main loop - thread begins procedure:
	// ...
	while (TRUE)
	{


		//1
		/* --------------------------------------------- */
		/* Attempt Priorities Queue resource Write lock  */
		/* --------------------------------------------- */
		if (WRITE_ACCESS_GRANTED != write_lock(p_params->p_prioritiesQueueLock)) {//==1
			printf("Queue access failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}
		//------------------locked

		//Find most prioritized remaining task size in bytes. 
		//If returned value is (ULONG_MAX-1), then the queue is empty -->> NO MORE TASKS TO SOLVE... EXIT..
		if ((ULONG_MAX - 1) == (taskInitialByteOffsetChosenByPriority = Top(p_params->p_prioritiesQueue))) { //remind the constants
			if (STATUS_CODE_SUCCESS != write_release(p_params->p_prioritiesQueueLock)) {
				printf("Queue lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
				closeHandleProcedure(p_h_tasksList);
				return STATUS_CODE_FAILURE;
			}
			break; // We reached an empty Queue...
		}

		if (STATUS_CODE_SUCCESS != Pop(p_params->p_prioritiesQueue)) {
			//Generally, there should be no reason for the thread to receive an output from Pop(.) which equals POP_QUEUE_WAS_EMPTY because, Top(.) would sense an empty queue first..
			if (STATUS_CODE_SUCCESS != write_release(p_params->p_prioritiesQueueLock)) {
				printf("Queue lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
				closeHandleProcedure(p_h_tasksList);
				return STATUS_CODE_FAILURE;
			}
			break; // We reached an empty Queue... previous Top-empty-queue case should suffice
		}

		//-----------------unlocking
		if (STATUS_CODE_SUCCESS != write_release(p_params->p_prioritiesQueueLock)) {
			printf("Queue lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}
		/* ----------------------------------------------------- */
		/* Release Attempt Priorities Queue resource Write lock  */
		/* ----------------------------------------------------- */





		//Access the Tasks file data struct for Reading (This resource is an ONLY reading resource which is why
		// it doesn't require any Synchronization)
		taskNumberStringNumberOfBytes = fetchMostPrioritizedTaskSizeInBytes(
			p_params->p_tasksFileData,				/* Task-list file data struct */
			taskInitialByteOffsetChosenByPriority	/* Initial byte offset from the beginning of the Tasks file */
		);






		//2
		/* ------------------------------------------- */
		/* Attempt Tasks list file resource Read lock  */
		/* ------------------------------------------- */
		if (READ_ACCESS_GRANTED != read_lock(p_params->p_tasksFileLock)) {//==1
			printf("Tasks file reading access failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}
		//------------------locked

		//Read most prioritized remaining task's value
		if (STATUS_CODE_FAILURE == fetchTaskByteOffsetByPriorityOrTaskItself( //Fetch Task-Itself
			p_h_tasksList,								/* a handle to the Tasks-list file */
			taskInitialByteOffsetChosenByPriority,		/* initial byte offset from the beginning of the Tasks file */
			taskNumberStringNumberOfBytes,				/* number of bytes that are used to represent the current task's line's string */
			&task										/* The task's variable address, for updating with the value computed in the function */
		)) { //p_currentPriorityCell->value == taskInitialByteOffsetChosenByPriority
			if (STATUS_CODE_SUCCESS != read_release(p_params->p_tasksFileLock)) {
				printf("Tasks file reading lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			}
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}

		//--------------unlocking
		if (STATUS_CODE_SUCCESS != read_release(p_params->p_tasksFileLock)) {
			printf("Tasks file reading lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}
		/* ------------------------------------------- */
		/* Release Tasks list file resource Read lock  */
		/* ------------------------------------------- */




		//Solve Task.....
		printf("Thread no. %lu beginning solving task: %lu...\n", GetCurrentThreadId(), task);
		if (NULL == (p_taskPrimeFactorsStringData = receivePrimeFactorizedListString(task))) {
			printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
			closeHandleProcedure(p_h_tasksList);
			return STATUS_CODE_FAILURE;
		}






		//3
		/* -------------------------------------------- */
		/* Attempt Tasks list file resource Write lock  */
		/* -------------------------------------------- */
		if (WRITE_ACCESS_GRANTED != write_lock(p_params->p_tasksFileLock)) {//==1
			printf("Tasks file writing access failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			freeTheString(p_taskPrimeFactorsStringData);
			return STATUS_CODE_FAILURE;
		}
		//------------------locked

		//Write the solution (Task's number prime factors string) to the Tasks list file 
		if (STATUS_CODE_FAILURE == printSolvedTaskToTasksListFile(p_h_tasksList, p_taskPrimeFactorsStringData->p_numberPrimeFactorsString, p_taskPrimeFactorsStringData->stringSize)) {
			//Releasing Tasks file writing lock, in case writing the solution to Tasks file failed to prevent Deadlock!
			if (STATUS_CODE_SUCCESS != write_release(p_params->p_tasksFileLock)) {
				printf("Tasks file writing lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			}
			closeHandleProcedure(p_h_tasksList);
			freeTheString(p_taskPrimeFactorsStringData);
			return STATUS_CODE_FAILURE;
		}

		//----------------unlocking
		if (STATUS_CODE_SUCCESS != write_release(p_params->p_tasksFileLock)) {
			printf("Tasks file writing lock release failed...\n"); // Might remove due to 'printf' being non-atomic, & when multiple Writer to queue reach timeout, they would all print this message == OVERLOAD CMD
			closeHandleProcedure(p_h_tasksList);
			freeTheString(p_taskPrimeFactorsStringData);
			return STATUS_CODE_FAILURE;
		}
		/* -------------------------------------------- */
		/* Release Tasks list file resource Write lock  */
		/* -------------------------------------------- */



		//Free the task prime factors composition string reperesentation struct & close Tasks list file handle
		freeTheString(p_taskPrimeFactorsStringData);

	}
	//Closing the Tasks file Handle that was unique to the current thread
	closeHandleProcedure(p_h_tasksList);
	//Thread operation completed successfuly....
	return STATUS_CODE_SUCCESS;
}



//......................................Static functions..........................................

static HANDLE* openFileForReadingAndWriting(char* p_filePath)
{
	HANDLE* p_h_fileHandle = NULL;
	//Assert
	assert(NULL != p_filePath);

	//Allocating dynamic memory (Heap) for a file Handle pointer
	if (NULL == (p_h_fileHandle = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a Handle to file '%s'.\n", p_filePath);
		printf("At file: %s\n at line number: %d\n at function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return  NULL;
	}

	//Open file for reading
	*p_h_fileHandle = CreateFile(
		p_filePath,							// Const null - terminated string describing the file's path
		GENERIC_READ | GENERIC_WRITE,		// Desired Access is set to Reading mode 
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// Share Mode:Here various threads may read from the input file (input message)
		NULL,								// No Security Attributes
		OPEN_EXISTING,						// The message is an existing file. If it doesn't exist we shouldn't open anything
		FILE_ATTRIBUTE_NORMAL,				// General reading in files 
		NULL								// No Template
	);
	//File Handle creation validation
	if (INVALID_HANDLE_VALUE == *p_h_fileHandle) {
		printf("Error: Failed to create a Handle to the Tasks file '%s' for tasks retreival, with code: %d.\n", p_filePath, GetLastError());
		free(p_h_fileHandle);
		return NULL;
	}
	//Returning a pointer to the created handle to file
	return p_h_fileHandle;
}

static DWORD fetchMostPrioritizedTaskSizeInBytes(file* p_tasksFileData, DWORD taskByteOffsetFromBeginningOfTheFile)
{
	line* p_currentLine = NULL;
	//Asserts
	assert(NULL != p_tasksFileData);
	assert(0 <= taskByteOffsetFromBeginningOfTheFile);

	//Set the current line data to be the first line data in the Tasks list file
	p_currentLine = p_tasksFileData->p_firstLineInFile;
	//Iterate through the Tasks list file data framed as lines structures & locate the number of bytes the task is composed of
	while (NULL != p_currentLine->p_nextLine) {
		if (taskByteOffsetFromBeginningOfTheFile == p_currentLine->initialPositionByByte)  return p_currentLine->numberOfBytesInLine;
		p_currentLine = p_currentLine->p_nextLine;
	}

	//If the while loop wasn't entered it means there was only one line with a task, so the first's line number of bytes is sent back
	//If the last cell (line) in the list is the current prioritized task, then the number of bytes is sent from here after the loop finished 
	return p_currentLine->numberOfBytesInLine;

}

static BOOL printSolvedTaskToTasksListFile(HANDLE* p_h_tasksListFile, LPTSTR p_numberPrimeFactorsString, DWORD stringLength)
{
	DWORD retValSet = 0, numberOfBytesWritten = 0;
	BOOL retValWrite = FALSE;
	//Asserts
	assert(p_h_tasksListFile != NULL);
	assert(p_numberPrimeFactorsString != NULL);


	//Set the Handle to point at the byte position from which the print to the file should start - end of file!!
	retValSet = SetFilePointer(
		*p_h_tasksListFile,					//Tasks List file Handle 
		0,									//Initial byte position set to be the begining of teh writing mark
		NULL,								//Assuming there is less than 2^32 bytes in the input file -recheck
		FILE_END							//Starting byte count is set to the end of the file which constatnly changes
	);
	//Validate Handle pointing succeeded...
	if (INVALID_SET_FILE_POINTER == retValSet) {
		//Initial byte position of wasn't found
		printf("Error: Failed to reset the file Handle pointer position for printing, with code: %d.\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Writing to Tasks List file at the desired position
	retValWrite = WriteFile(
		*p_h_tasksListFile,						//Tasks List Handle after being set at the desired byte-position 
		p_numberPrimeFactorsString,				//Task's (Number) prime factors string buffer pointer
		stringLength,							//Total number of bytes, needed to be written, of the thread's task in the file
		&numberOfBytesWritten,					//Pointer to the total number of bytes written 
		NULL									//Overlapped - off
	);
	if (STATUS_FILE_WRITING_FAILED == retValWrite) {
		//Failed to write the needed memory from the file
		printf("Error: Failed to write to the file Handle. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//ATTEMPT
	if (STATUS_NEW_END_OF_FILE_SET_FAILED == SetEndOfFile(*p_h_tasksListFile)) {
		printf("Error: Failed to write to the file Handle. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Writing to output file was successful...
	return STATUS_CODE_SUCCESS;
}