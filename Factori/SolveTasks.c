/* SolveTasks.c
-----------------------------------------------------------------------------------------
	Module Description - This module contains functions meant for initializing the 
		multi-threading operation, by arranging the Tasks Priorities contents into a 
		queue & assembling a threadPacket struct that contains resources pointers and 
		resources locks pointers, creating a set of IDs, creating threads that execute 
		the TasksSolverThread routine, setting a waiting scheme (with unique timeout) &
		validating the threads exit codes...
-----------------------------------------------------------------------------------------
*/

// Library includes ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes --------------------------------------------------------------------
#include "SolveTasks.h"

// Constants
static const BOOL  STATUS_CODE_FAILURE = FALSE;
static const BOOL  STATUS_CODE_SUCCESS = TRUE;

static const DWORD SINGLE_OBJECT = 1;



static const BOOL STATUS_FILE_READING_FAILED = (BOOL)0;

static const BOOL WRITERS_ONLY = 1;

static const int   DEFAULT_THREAD_STACK_SIZE = 0;

	//WaitForMultipleObjects
static const BOOL  WAIT_FOR_ALL_OBJECTS = TRUE;
static const DWORD TIMEOUT_BASE_CONSTANT_MS = 10; //10 milli-seconds

	//Recive exit codes
static const BOOL  GET_EXIT_CODE_FAILURE = 0;
static const BOOL  THREAD_PROCESSED_SUCCESSFULY = TRUE;




// Functions decleraitions ---------------------------------------------------------------
/// <summary>
///  Description - This function allocates (on the Heap - dynamic mem.) memory for a Handle and 
///		uses CreateFile WINAPI function to create a Handle to the Tasks Priorities list file (with Reading permission only).
///		It returns a pointer to newly created file Handle.
/// </summary>
/// <param name="char* p_filePath - A pointer to the commandline argument (string) that represents the Tasks Priorities list file's path (absolute\relative)"></param>
/// <returns>A pointer to a Handle to the Tasks Priorities file allocated on the heap if successful, or NULL if failed</returns>
static HANDLE* openTasksPrioritiesFileForReading(char* p_filePath);
/// <summary>
///	 Description - This function receives the Tasks Priorities list file data ("framing" of the file into lines-byte-offsets), constructs
///		a "queue" struct, reads the Tasks' initial byte-positions values (values placed in the Tasks Priorities file) one at a time, and places every value 
///		as an element("cell") in the queue. Finally, the function returns a pointer to the constructed queue struct.  
/// </summary>
/// <param name="char* p_tasksPrioritiesListFilePath - A pointer to the commandline argument (string) that represents the Tasks Priorities list file's path (absolute\relative)"></param>
/// <param name="file* p_tasksPrioritiesFileData - A pointer to the Tasks Priorities list file data struct assembled with the file's lines' byte-offsets"></param>
/// <param name="int numberOfTasks - An integer that represents the number of tasks the program needs to solve"></param>
/// <returns>A pointer to a filled Queue struct with no. of  "cell"s equals #Tasks, each holds a task's initial byte-position within the Tasks file, if successful, or NULL if failed</returns>
static queue* buildTasksPrioritiesQueue(char* p_tasksPrioritiesListFilePath, file* p_tasksPrioritiesFileData, int numberOfTasks);
/// <summary>
///	 Description - This function creates all of a resource's lock's Synchronous Objects by firstly allocating memory (on the Heap) for every Handle to every object,
///		and then, creates the objects with CreateMutex and CreateSemaphore WINAPI functions and attaches them to the their Handles. 
///		Also the function makes a descision based on the a Writers-ONLY flag indicating whether the resource will be accessed only by Writers (In the case
///		of the resource accessed by Readers only, the resource WOULDN'T NEED A LOCK) - If the bit is '1', then only a Turnstile Mutex is created and attached
///		to a Handle pointed to by the Turnstil Mutex handle pointer field in the lock struct. 
/// </summary>
/// <param name="lock* p_lock - A pointer to 'lock' struct"></param>
/// <param name="int writersOnlyFlag - An integer indicating whether the resource associated with the lock is a Writers-ONLY resource or not"></param>
/// <returns>A BOOL value representing the function's outcome (Lock struct's fields were updated with Synchronous objects). Success (TRUE) or Failure (FALSE)</returns>
static BOOL createSynchronousObjects(lock* p_lock, int writersOnlyFlag);
/// <summary>
///  Description - This function allocates memory (on the Heap) for a "threadPackage" struct that contains pointers to all relevant resources and pointers to these
///		resources' locks. It also allocates memory for these "lock"s structs. This function updates all relevant fields of the "threadPackage" struct with the input
///		resources pointers and calls a Synchronous Objects creation procedure to contruct the locks as needed (Writer\Readers, Writers-ONLY) and updates
///		the pointers to these locks accordingly.
/// </summary>
/// <param name="char* p_tasksListFilePath - A pointer to the commandline argument (string) that represents the Tasks list file's path (absolute\relative)"></param>
/// <param name="p_tasksPrioritiesQueue"></param>
/// <param name="p_tasksFileData"></param>
/// <param name="int numberOfTasks - An integer that represents the number of tasks the program needs to solve"></param>
/// <param name="int numberOfThreads - An integer that represents the number of threads the program will use for the tasks solving procedure."></param>
/// <returns>A pointer to a filled "threadPackage" struct, if successful, or NULL if failed</returns>
static threadPackage* initializeThreadsParametersStruct(char* p_tasksListFilePath, queue* p_tasksPrioritiesQueue,
	file* p_tasksFileData, int numberOfTasks, int numberOfThreads);
/// <summary>
/// Description - This function creates a new thread and attaches a Handle to it. It channels the thread a unique "threadPackage" struct
///		and a unique ID from the threadIds array.The threads are set to have a default stack size(code - 0)
///		and a default security descriptor(Sec.Att. - null).
/// </summary>
/// <param name="LPTHREAD_START_ROUTINE p_startRoutine - A pointer to the thread's routine address"></param>
/// <param name="LPVOID p_threadParameters - A void pointer to the unqiue 'threadPackage' data struct"></param>
/// <param name="LPDWORD p_threadId - A pointer to ID's address"></param>
/// <returns>A Handle to the thread if successful, or NULL if failed (validated outside of this function)</returns>
static HANDLE createThreadSimple(LPTHREAD_START_ROUTINE p_startRoutine, LPVOID p_threadParameters, LPDWORD p_threadId);
/// <summary>
///  Description - This function receives a pointer to a Handles array and activates WaitForMultipleObjects. It validates
///		whether all threads terminated on time(According to the time cap), or if anything else failed.
/// </summary>
/// <param name="HANDLE* p_threadHandles - A pointer the threads Handles array."></param>
/// <param name="int numberOfThreads - The number of threads that will assist in calculating the waiting timeout"></param>
/// <param name="int numberOfTasks - An integer that represents the number of tasks that will assist in calculating the waiting timeout"></param>
/// <returns>A BOOL value representing the function's outcome (wait code is WAIT_OBJECT_0). Success (TRUE) or Failure (FALSE)</returns>
static BOOL validateThreadsWaitCode(HANDLE* p_threadHandles, int numberOfThreads, int numberOfTasks);
/// <summary>
///  Description - This function receives a pointer to a Handles array and validates whether the threads terminated
///		on time(meaning, before WaitForMultipleObject's time cap reached) and that their exit codes are as expected of
/// 	threads that have completed successfuly their processing and printing operations of the Tasks they fetched.
/// </summary>
/// <param name="HANDLE* p_threadHandles - A pointer the threads Handles array."></param>
/// <param name="int numberOfThreads - The number of threads is also the size of the Handles array"></param>
/// <returns>A BOOL value representing the function's outcome (All exit codes are legitimate and all threads terminated on time). Success(TRUE) or Failure(FALSE) </returns>
static BOOL validateThreadsExitCodes(HANDLE* p_threadHandles, int numberOfThreads);






//SEARCH 'CMD'

// Functions definitions ---------------------------------------------------------------

BOOL solveTasks(char* p_tasksListFilePath, char* p_tasksPrioritiesListFilePath, int numberOfTasks, int numberOfThreads)
{
	file* p_tasksPrioritiesFileData, *p_tasksFileData;
	queue* p_tasksPrioritiesQueue;
	HANDLE* p_threadHandles = NULL; //for WaitForMultipleObjects this handle array should be a constant, but it's size can't be constant.... - evantually it works fine - don't know why!
	LPDWORD p_threadIds = NULL;
	threadPackage* p_allThreadsParam = NULL;
	int th = 0;
	//Input integrity validation
	if ((NULL == p_tasksListFilePath) || (NULL == p_tasksPrioritiesListFilePath) || (0 > numberOfTasks) || (0 >= numberOfThreads)) { 
		printf("Error: Bad inputs to function: %s\n", __func__); return STATUS_CODE_FAILURE;
	}
	
	//Frame the Tasks Priorities List file into lines structures of bytes boundries, named tasksPrioritiesFileStruct, by performing a single initial run
	if (NULL == (p_tasksPrioritiesFileData = frameFileLines(p_tasksPrioritiesListFilePath)))  		return STATUS_CODE_FAILURE;

	//Use tasksPrioritiesFileStruct to read all offsets into Queue (in later steps), or read one-by-one for step 1
	if (NULL == (p_tasksPrioritiesQueue = buildTasksPrioritiesQueue(p_tasksPrioritiesListFilePath, p_tasksPrioritiesFileData, numberOfTasks))) return STATUS_CODE_FAILURE;

	//Frame the Tasks List file into lines structures of bytes boundries, named tasksFileStruct, by performing a single initial run
	if (NULL == (p_tasksFileData = frameFileLines(p_tasksListFilePath))) { DestroyQueue(&p_tasksPrioritiesQueue); return STATUS_CODE_FAILURE; }


	//Allocate memory for threads handles - #Handles = #Threads
	if (NULL == (p_threadHandles = (HANDLE*)malloc(sizeof(HANDLE) * numberOfThreads))) {
		printf("Error: Failed to allocate memory for a Handle array.\n");
		printf("At file: %s\n at line number: %d\n at function: %s\n\n", __FILE__, __LINE__, __func__);
		//Add cleanup for Queue & Tasks file Data struct
		freeTheFile(p_tasksFileData);
		DestroyQueue(&p_tasksPrioritiesQueue);
		return STATUS_CODE_FAILURE;
	}

	//Allocate memory for the threads handles' IDs
	if (NULL == (p_threadIds = (LPDWORD)malloc(sizeof(DWORD) * numberOfThreads))) {
		printf("Error: Failed to allocate memory for a Handles' IDs.\n");
		printf("At file: %s\n at line number: %d\n at function: %s\n\n", __FILE__, __LINE__, __func__);
		free(p_threadHandles);
		//Add cleanup for Queue & Tasks file Data struct
		freeTheFile(p_tasksFileData);
		DestroyQueue(&p_tasksPrioritiesQueue);
		return STATUS_CODE_FAILURE;
	}
	

	if (NULL == (p_allThreadsParam = initializeThreadsParametersStruct(
		p_tasksListFilePath,				/* pointer to the Tasks list text file path */
		p_tasksPrioritiesQueue,				/* pointer to the constructed Tasks Priorities Queue */
		p_tasksFileData,					/* pointer to a file data struct describing the Tasks list file */
		numberOfTasks,						/* # Tasks to solve */
		numberOfThreads						/* # Threads to use for solving tasks */
	))) {
		printf("Error: Failed to construct the threads' parameters block.\n");
		free(p_threadHandles);
		free(p_threadIds);
		//Add cleanup for Queue & Tasks file Data struct -> Added inside
		return STATUS_CODE_FAILURE;
	}
	
	


	
	
	

	
	
	

	
	printf("\nCommence Tasks solving......\n");
	//Begin threads creation loop...
	for (th; th < numberOfThreads; th++) {




		//Create thread!
		*(p_threadHandles + th) = createThreadSimple(
			(LPTHREAD_START_ROUTINE)tasksSolverThread,	/* thread routine */
			p_allThreadsParam,							/* thread's parameters struct */
			p_threadIds + th							/* thread ID address */
		);
		//Validate thread no. 'th' handle
		if (NULL == *(p_threadHandles + th)) {
			printf("Error: Failed to create thread no. %d, with code: %d.\n", th, GetLastError());
			printf("At file: %s\n at line number: %d\n at function: %s\n", __FILE__, __LINE__, __func__);
			freeTheThreadPackage(p_allThreadsParam);
			closeThreadsProcedure(p_threadHandles, p_threadIds, th); //th == #Threads initiated so far!
			return STATUS_CODE_FAILURE;
		}
	}



	//Wait for all threads to complete decrypting\encrypting & printing to the output message file
	if (STATUS_CODE_FAILURE == validateThreadsWaitCode(p_threadHandles, numberOfThreads, numberOfTasks)) {
		freeTheThreadPackage(p_allThreadsParam);
		closeThreadsProcedure(p_threadHandles, p_threadIds, numberOfThreads);
		return STATUS_CODE_FAILURE;
	}




	//Exit code validation
	if (STATUS_CODE_FAILURE == validateThreadsExitCodes(p_threadHandles, numberOfThreads)) {
		freeTheThreadPackage(p_allThreadsParam);
		closeThreadsProcedure(p_threadHandles, p_threadIds, numberOfThreads);
		return STATUS_CODE_FAILURE;
	}



	//Reaching here means the following:
	//	All threads terminated on time
	//	All threads completed successfully solving tasks
	freeTheThreadPackage(p_allThreadsParam);
	closeThreadsProcedure(p_threadHandles, p_threadIds, numberOfThreads);


	//Multi-threading operation succeeded.....
	return STATUS_CODE_SUCCESS;
}





//This function is placed in this module because it is used both in this module(SolveTasks.c) & in TasksSolverThread.c module
//It is not placed in the latter because, this function is first used here, to create the Tasks Priorities Queue by reading from
// the Tasks Prioirities list file. Following the Queue's construction, and the Threads' creation, the Threads' routing make use of it...
BOOL fetchTaskByteOffsetByPriorityOrTaskItself(HANDLE* p_h_fileHandle, DWORD initialByteOffset, DWORD numberOfBytesToRead, LPDWORD translatedNumber)
{
	LPTSTR p_numericString = NULL;
	errno_t retVal;
	DWORD retValSet = 0, numberOfBytesRead = 0;
	BOOL retValRead = FALSE;
	//Input integrity validation
	if ((NULL == p_h_fileHandle)  || (0 > initialByteOffset) || (0 > numberOfBytesToRead) || (NULL == translatedNumber)) {
		printf("Error: Bad inputs to function: %s\n", __func__); return STATUS_CODE_FAILURE;
	}

	//Firstly, setting the file Handle to the initial byte position of the line
	retValSet = SetFilePointer(
		*p_h_fileHandle,		//Input file Handle 
		initialByteOffset,		//Initial byte position set to be the begining of the line
		NULL,					//Assuming there is less than 2^32 bytes in the input file -recheck
		FILE_BEGIN				//Starting byte count is set to the start of the file (byte=0)
	);
	//Validate Handle pointing succeeded...
	if (INVALID_SET_FILE_POINTER == retValSet) {
		//Initial byte position of the line wasn't found
		printf("Error: Failed to reset the file Handle pointer position for reading, with code: %d.\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}
	//Secondly, Memory allocation for the string buffer containing the line's characters
	if ((p_numericString = (LPTSTR)calloc(sizeof(TCHAR), numberOfBytesToRead + 1)) == NULL) {
		printf("Error: Failed to allocate memory for the section's string buffer.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}
	//Thirdly, reading from the file Handle the size of the line in bytes
	retValRead = ReadFile(
		*p_h_fileHandle,			//File Handle after being set at the desired byte-positions
		p_numericString,			//Task byte-offset (from priority list) or Task itself string buffer pointer
		numberOfBytesToRead,		//Total number of bytes, needed to be read
		&numberOfBytesRead,			//Pointer to the total number of bytes fetched 
		NULL						//Overlapped - off
	);
	if (STATUS_FILE_READING_FAILED == retValRead) {
		//Failed to read the needed memory from the file
		printf("Error: Failed to read from the file Handle. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		free(p_numericString);
		return STATUS_CODE_FAILURE;
	}

	//Lastly, translating the numeric string, representing either the task's byte-offset fetched from the Tasks Priority
	// list file or the task itself fetch from the Tasks list file, to a DWORD (lu - unsigned long) data type number.
	//		DWORD is taken as datatype since we are assured the Priority values & Tasks values are postive....
	if ((retVal = sscanf_s(p_numericString, "%lu", translatedNumber)) == EOF) {
		free(p_numericString);
		printf("Error: Failed to translate the current task's numeric string or byte offset. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Free numeric string
	free(p_numericString);
	//The desired number address was successfully updated with the read & translated value...
	return STATUS_CODE_SUCCESS;
}








//......................................Static functions.......................................... 


static HANDLE* openTasksPrioritiesFileForReading(char* p_filePath)
{
	HANDLE* p_h_fileHandle = NULL;
	//Assert
	assert(NULL != p_filePath);

	//Allocating dynamic memory (Heap) for a file Handle pointer
	if (NULL == (p_h_fileHandle = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a Handle to file '%s'.\n", p_filePath);
		printf("At file: %s\n at line number: %d\n at function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}


	//Open file for reading
	*p_h_fileHandle = CreateFile(
		p_filePath,				// Const null - terminated string describing the file's path
		GENERIC_READ,			// Desired Access is set to Reading mode 
		FILE_SHARE_READ,		// Share Mode:Here various threads may read from the input file (input message)
		NULL,					// No Security Attributes
		OPEN_EXISTING,			// The message is an existing file. If it doesn't exist we shouldn't open anything
		FILE_ATTRIBUTE_NORMAL,	// General reading in files 
		NULL					// No Template
	);
	//File Handle creation validation
	if (INVALID_HANDLE_VALUE == *p_h_fileHandle) {
		printf("Error: Failed to create a Handle to Tasks Priorities file '%s' for Queue creation, with code: %d.\n", p_filePath, GetLastError());
		free(p_h_fileHandle);
		return NULL;
	}
	//Returning a pointer to the created handle to file
	return p_h_fileHandle;
}

static queue* buildTasksPrioritiesQueue(char* p_tasksPrioritiesListFilePath, file* p_tasksPrioritiesFileData, int numberOfTasks)
{
	queue* p_tasksPrioritiesQueue = NULL;
	HANDLE* p_h_tasksPrioritiesListFile = NULL;
	line* p_currentPriorityLine = NULL;
	int taskCount = 0;
	DWORD taskOffsetChosenByPriority = 0;
	//Input integrity validation
	if ((NULL == p_tasksPrioritiesListFilePath) || (NULL == p_tasksPrioritiesFileData)) {
		printf("Error: Bad inputs to function: %s\n", __func__); return NULL;
	}

	//Construct the Queue struct with dynamic memory allocation (Queue Initialization)
	if (NULL == (p_tasksPrioritiesQueue = InitializeQueue())) {
		freeTheFile(p_tasksPrioritiesFileData);
		return NULL;
	}

	//Opening a Handle to the input tasks priorities list file
	if (NULL == (p_h_tasksPrioritiesListFile = openTasksPrioritiesFileForReading(p_tasksPrioritiesListFilePath))) {
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		freeTheFile(p_tasksPrioritiesFileData);
		if (STATUS_CODE_SUCCESS != DestroyQueue(&p_tasksPrioritiesQueue)) printf("Failed to destroy queue...\n");
		return NULL;
	}


	//Setting the most prioritized task byte-offset line data as the current analyzed line
	p_currentPriorityLine = p_tasksPrioritiesFileData->p_firstLineInFile;

	//Begin extracting tasks' initial offsets & pusing them to the queue...
	for (taskCount; taskCount < numberOfTasks; taskCount++) {

		//Read most prioritized remaining task's byte-offset by the main thread & insert it to taskOffsetChosenByPriority
		if (STATUS_CODE_FAILURE == fetchTaskByteOffsetByPriorityOrTaskItself(p_h_tasksPrioritiesListFile, p_currentPriorityLine->initialPositionByByte,
			p_currentPriorityLine->numberOfBytesInLine, &taskOffsetChosenByPriority)) {
			closeHandleProcedure(p_h_tasksPrioritiesListFile);
			freeTheFile(p_tasksPrioritiesFileData);
			if (STATUS_CODE_SUCCESS != DestroyQueue(&p_tasksPrioritiesQueue)) printf("Failed to destroy task priorities queue...\n");
			return NULL;
		}

		//Add another cell to the end of the queue with the value inserted to taskOffsetChosenByPriority,
		// which represents the task placed at the current position in the waiting-queue... (Queue Push)
		if (STATUS_CODE_FAILURE == Push(p_tasksPrioritiesQueue, taskOffsetChosenByPriority)) {
			closeHandleProcedure(p_h_tasksPrioritiesListFile);
			freeTheFile(p_tasksPrioritiesFileData);
			//Queue already destroyed within Push(.)
			return NULL;
		}


		//Next task byte offset chosen by priority retrieval from taks priority file....
		if (NULL != p_currentPriorityLine->p_nextLine) p_currentPriorityLine = p_currentPriorityLine->p_nextLine;
	}

	//Close file Handle & free the file-data of the Tasks Priorities list file (It has no need anymore!!!)
	closeHandleProcedure(p_h_tasksPrioritiesListFile);
	freeTheFile(p_tasksPrioritiesFileData);

	//Return the Queue which now holds in the order of the tasks-execution-order 
	// the bytes-offsets of the tasks in the Tasks list file...
	printf("Queue created & was updated with values from the Tasks Priorities list file.\n");
	return p_tasksPrioritiesQueue;
}





static BOOL createSynchronousObjects(lock* p_lock, int writersOnlyFlag)
{
	//Assert
	assert(NULL != p_lock);


	//Allocating dynamic memory (Heap) for a lock's Turnstile Mutex Handle 
	if (NULL == (p_lock->p_h_turnstileMutex = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a resource's lock's Turnstile Mutex handle.\n");
		printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
		closeHandleProcedure(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
		return  STATUS_CODE_FAILURE;
	}

	//Creating a un-named Mutex, since the pointer to the only Handle of this Mutex will be sent to all threads with the Lock
	*(p_lock->p_h_turnstileMutex) = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == *(p_lock->p_h_turnstileMutex)) {
		printf("Error: Failed to create a handle to lock's Turnstile Mutex with code: %d.\n", GetLastError());
		free(p_lock->p_h_turnstileMutex);
		return  STATUS_CODE_FAILURE;
	}


	//If the resource is defined to expect only one kind of threads accessing it - writers - then we may forfeit 
	//	the other synchronous objects (1 Mutex will do the job)
	if (WRITERS_ONLY != writersOnlyFlag) {
		//Allocating dynamic memory (Heap) for a lock's Readers-counting-procedure Mutex Handle 
		if (NULL == (p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
			printf("Error: Failed to allocate memory for a resource's lock's Readers-counting-procedure Mutex handle.\n");
			printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
			return  STATUS_CODE_FAILURE;
		}

		//Creating a un-named Mutex, since the pointer to the only Handle of this Mutex will be sent to all threads with the Lock
		*(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex) = CreateMutex(
			NULL,	/* default security attributes */
			FALSE,	/* initially not owned */
			NULL);	/* unnamed mutex */
		if (NULL == *(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex)) {
			printf("Error: Failed to create a handle to lock's Readers-counting-procedure Mutex with code: %d.\n", GetLastError());
			closeHandleProcedure(p_lock->p_h_turnstileMutex);
			free(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
			return  STATUS_CODE_FAILURE;
		}




		//Allocating dynamic memory (Heap) for a lock's resource vacancy status Semaphore Handle 
		if (NULL == (p_lock->p_h_vaccantResourceStatusSemphore = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
			printf("Error: Failed to allocate memory for a resource's lock's vacancy status Semaphore handle.\n");
			printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
			closeHandleProcedure(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
			closeHandleProcedure(p_lock->p_h_turnstileMutex);
			return  STATUS_CODE_FAILURE;
		}


		//Creating Sephamore for Readers documentation & signalling to a Write\Reader when the resource is vacanat completely
		*(p_lock->p_h_vaccantResourceStatusSemphore) = CreateSemaphore(
			NULL,					/*  default security attributes - no need that the thread can be inherited by child processes */
			SINGLE_OBJECT,			/*  initial count - setting this to '1' so whatever thread (Reader\Writer) that requests access to the resource first, will be granted with approval */
			SINGLE_OBJECT,			/*  maximum count - setting this to '1' so only one type of thread - Reader\Writer will be able to use the signal & access the resource at a time*/
			NULL					/*  semaphore name - un-named because the Semaphore's handle is created on the Heap & is transferred to all threads */
		);
		if (NULL == *(p_lock->p_h_vaccantResourceStatusSemphore)) {
			printf("Error: Failed to create a handle to lock's Readers-counting-procedure Mutex with code: %d.\n", GetLastError());
			closeHandleProcedure(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
			closeHandleProcedure(p_lock->p_h_turnstileMutex);
			free(p_lock->p_h_vaccantResourceStatusSemphore);
			return  STATUS_CODE_FAILURE;
		}
	}

	//Lock's Synchronous objects memory allocation & creation succeeded..
	return STATUS_CODE_SUCCESS;
}

static threadPackage* initializeThreadsParametersStruct(char* p_tasksListFilePath, queue* p_tasksPrioritiesQueue,
	file* p_tasksFileData, int numberOfTasks, int numberOfThreads)
{
	threadPackage* p_allThreadsParam = NULL;

	//Asserts
	assert(NULL != p_tasksListFilePath);
	assert(NULL != p_tasksPrioritiesQueue);
	assert(NULL != p_tasksFileData);

	//Allocate dynamic memory for a threadPackage struct which will in fact be a single parameters struct, 
	// that all threads will receive (No Duplicates!), since it contains the resources' pointers & their locks pointers,
	// which do not require uniqueness per thread
	if (NULL == (p_allThreadsParam = (threadPackage*)calloc(sizeof(threadPackage), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for threads' parameters struct.\n");
		printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
		freeTheFile(p_tasksFileData);
		DestroyQueue(&p_tasksPrioritiesQueue);
		return  NULL;
	}

	//Direct the threads' package's fields to the same addresses as the input pointer point at
	p_allThreadsParam->p_tasksFilePath = p_tasksListFilePath;
	p_allThreadsParam->p_prioritiesQueue = p_tasksPrioritiesQueue;
	p_allThreadsParam->p_tasksFileData = p_tasksFileData;

	//Allocate memory for Tasks file lock & the Priorities Queue lock
	if (NULL == (p_allThreadsParam->p_tasksFileLock = (lock*)calloc(sizeof(lock), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for Tasks' file lock struct.\n");
		printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
		freeTheThreadPackage(p_allThreadsParam);
		return  NULL;
	}
	if (NULL == (p_allThreadsParam->p_prioritiesQueueLock = (lock*)calloc(sizeof(lock), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for Priorities Queue lock struct.\n");
		printf("At file: %s\n at line number: %d\n at function: %s", __FILE__, __LINE__, __func__);
		freeTheThreadPackage(p_allThreadsParam);
		return  NULL;
	}

	//Allocate memory for Syncronous objects for every lock
	if (STATUS_CODE_FAILURE == createSynchronousObjects(p_allThreadsParam->p_tasksFileLock, WRITERS_ONLY - 1)) { //WRITERS & READERS ARE DEFINED TO THE RESOURCE
		printf("Error: Failed to create the Tasks' file lock with its Sync. objects\n");
		freeTheThreadPackage(p_allThreadsParam);
		return NULL;
	}
	if (STATUS_CODE_FAILURE == createSynchronousObjects(p_allThreadsParam->p_prioritiesQueueLock, WRITERS_ONLY)) { //WRITERS 
		printf("Error: Failed to create the Priorities Queue lock with its Sync. objects\n");
		freeTheThreadPackage(p_allThreadsParam);
		return NULL;
	}
	//Update locks timeouts variables
	p_allThreadsParam->p_tasksFileLock->numberOfTasks = numberOfTasks;
	p_allThreadsParam->p_tasksFileLock->numberOfThreads = numberOfThreads;
	p_allThreadsParam->p_prioritiesQueueLock->numberOfTasks = numberOfTasks;
	p_allThreadsParam->p_prioritiesQueueLock->numberOfThreads = numberOfThreads;

	//Building the threads' parameters struct was successful
	return p_allThreadsParam;
}





static HANDLE createThreadSimple(LPTHREAD_START_ROUTINE p_startRoutine,
	LPVOID p_threadParameters,
	LPDWORD p_threadId)
{
	HANDLE h_threadHandle;

	//Asserts
	assert(NULL != p_startRoutine);
	assert(NULL != p_threadId);
	

	//Thread creation
	h_threadHandle = CreateThread(
		NULL,										 /*  default security attributes - no need that the thread can be inherited by child processes */
		DEFAULT_THREAD_STACK_SIZE,					 /*  use default stack size - most of the objects (e.g. section struct) are allocated in the heap instead */
		p_startRoutine,								 /*  thread function - remember it is an address! */
		p_threadParameters,							 /*  argument to thread function - this will be a section struct */
		0,											 /*  use default creation flags - thread runs immediately when it is created */
		//NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,/*  use normal priority flag & open new console for every thread. */
		p_threadId);								 /*  returns the thread identifier - thread ID */


	//Returning handle
	return h_threadHandle;
}


static BOOL validateThreadsWaitCode(HANDLE* p_threadHandles, int numberOfThreads, int numberOfTasks)
{
	DWORD waitCode = 0;
	//Asserts
	assert(NULL != p_threadHandles);
	assert(0 < numberOfThreads);

	//Wait for 3 seconds for all threads to finish...
	waitCode = WaitForMultipleObjects(
		numberOfThreads,									  						// number of objects in array
		p_threadHandles,															// array of objects
		WAIT_FOR_ALL_OBJECTS,														// wait for any object
		TIMEOUT_BASE_CONSTANT_MS*MAX_LENGTH_TASK*numberOfTasks/numberOfThreads);	// 0.01-second base wait  T.O.=func(#Tasks,#Threads,Max Task's length)
	 
	//Validating that all threads have finished....
	switch (waitCode) {
	case WAIT_OBJECT_0:
		printf("\nAll threads terminated on time.\nProceed to validate correctness of the exit codes of the threads...\n"); return STATUS_CODE_SUCCESS;
	case WAIT_TIMEOUT:
		printf("\nTIMEOUT: Not all threads terminated on time... \nSolving various tasks may have failed.\n"); return STATUS_CODE_FAILURE;
	case WAIT_ABANDONED_0:
		printf("\nAt least one of the objects is an abandoned Mutex object... \nEXITING....\n"); return STATUS_CODE_FAILURE;
	default:
		printf("0x%x\nExtended error code: %d\n", waitCode, GetLastError());
		return STATUS_CODE_FAILURE;
	}
}


static BOOL validateThreadsExitCodes(HANDLE* p_threadHandles, int numberOfThreads)
{
	int t = 0;
	BOOL retValGetExitCode = FALSE;
	DWORD exitCode = 0;
	//Asserts
	assert(NULL != p_threadHandles);
	assert(0 < numberOfThreads);

	//Exit code validation - main loop iterating all thread Handles
	for (t = 0; t < numberOfThreads; t++) {

		//Fetch the exit code & the GetExitCodeThread function return value of the t'th thread
		retValGetExitCode = GetExitCodeThread(*(p_threadHandles + t), &exitCode);

		//Validate the thread terminated correctly
		if (GET_EXIT_CODE_FAILURE == retValGetExitCode) { //add case for a thread which is STILL_ACTIVE
			printf("Error when getting thread no. %d exit code, with code: %d\n", t, GetLastError());
			return STATUS_CODE_FAILURE;
		}
		//Validate the thread terminated completely and no longer running
		if (STILL_ACTIVE == exitCode) { 
			printf("Thread no. %d is still alive after threads termination timeout... exiting.\n", t);
			return STATUS_CODE_FAILURE;
		}
		//Validating the thread's return value(exit code) is as expected of a thread that completed its' operation successfuly...
		if (THREAD_PROCESSED_SUCCESSFULY != exitCode) {
			printf("Exit codes indicating the taks solving procedure failed... exiting.\n");
			return STATUS_CODE_FAILURE;
		}
	}

	//All threads terminated & finished processing the message successfuly...
	printf("All exit codes indicate the threads completed their operation as needed\n\n\n");
	return STATUS_CODE_SUCCESS;
	
}