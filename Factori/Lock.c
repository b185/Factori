/* Lock.c
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
#include "Lock.h"


// Constants -----------------------------------------------------------------------------
static const BOOL  STATUS_CODE_FAILURE = FALSE;
static const BOOL  STATUS_CODE_SUCCESS = TRUE;

static const DWORD SINGLE_OBJECT = 1;

static const DWORD TIMEOUT_BASE_CONSTANT_MS = 1; //1 milli-second
static const DWORD RESOURCE_IS_NOT_EMPTY = 1;
static const DWORD RESOURCE_IS_EMPTY = 0;

static const DWORD READ_ACCESS = 1;
static const DWORD RELEASE_TURNSTILE_MUTEX_FAILED = 0;
static const DWORD RELEASE_EMPTY_RESOURCE_SEMAPHORE_FAILED = 0;


// Global Variables
static int numberOfRealTimeReadersCounter = 0;



// Functions declerations ------------------------------------------------------------------

/// <summary>
///  Description - This function performs the WaitForSingleObject(.) WINAPI function with a given timeout value, in order to receive ownership\signalling of
///		a Mutex\(Binary)Semaphore synchronous object. In case of timeout the entire program is said to have failed to solve all tasks in a proper matter, and 
///		the program will initiate exitting procedure.
/// </summary>
/// <param name="HANDLE* p_h_objectHandle - A pointer to Synchronous Object's handle"></param>
/// <param name="DWORD timeout - An unsigned integer value representing the timeout duration until WaitForSingleObject(.) should stop waiting"></param>
/// <param name="DWORD readOrWriteState - An unsigned integer indicating whether the lock is requested for a Reading request or Writing request, and helps choosing a suitable failure message"></param>
/// <returns>A BOOL value representing the function's outcome (Syncronous object "capturing"). Success (TRUE) or Failure (False) e.g. timeout or abondonment</returns>
static BOOL waitForSynchronousObjectToSignalOrUnlock(HANDLE* p_h_objectHandle, DWORD timeout, DWORD readOrWriteState);


// Functions definitions ------------------------------------------------------------------

lock* InitializeLock()
{
	lock* p_lock = NULL;
	//Lock struct dynamic memory allocation
	if (NULL == (p_lock = (lock*)calloc(sizeof(lock), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a Queue struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Memory allocation for the Lock struct has been successful
	return p_lock;
}

BOOL read_lock(lock* p_lock)
{
	//Input integrity validation
	if (NULL == p_lock) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}

	//Obtain Ownership of the Turnstile mutex that chooses between a new Reader to a new Writer...(even when there might be Readers inside the resource)
	//Explanation: Basically I had a hard time understding whether this part should remain outside of the read_lock function or within it,
	//	since, read_lock must always return with a positive result, if there are already Reader\s accessing the resource (Meaning,
	//	the file has already been locked for reading). But in the case a new Writer has arrived and OWNED the Turnstile mutex, then, new Readers
	//  would remain waiting from accessing the resource for READING, EVEN if there are already Readers inside the resource as contrary to the
	//	multiple Readers constraint, and when their waiting timeout reached the thread would quit with a fail message.
	//  Finally, I decided to insert the Turnstile mutex aquisition inside the read_lock function, as part of a "Load Balancing" regime 
	//	(between Writer & Readers) which is in fact a RACE to own the Turstile mutex lock in order to prevent STARVATION, even though
	//  a new Reader might have to wait when the resource is locked for reading (and there are Readers reading the resource), and would have
	//	to wait until the Writer owning the Turstile mutex will receive the "EmptyRoom" Semaphore signalling, enter the resource,
	//	write its' task, leave and signal back the Semaphore & Turstile mutex
	// Differently, leaving the Turstile mutex aquisition outside this function (Meaning, in the thread's regime), would statisfy the description
	//  completely ("If the resource is locked for reading, so read_lock will always return without any wait, for a new reader"), but wouldn't
	//  be elegant and would demand a programmer using the Lock.c module to always program this part as well..  I hope its' fine..
	if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
		p_lock->p_h_turnstileMutex,															//Mutex handle pointer
		TIMEOUT_BASE_CONSTANT_MS * MAX_LENGTH_TASK * p_lock->numberOfThreads,				//Timeout = (Max task length)*#Threads milliSeconds * 2
		READ_ACCESS																			//Reading bit, for printing appropriate failure message if needed
	))   return STATUS_CODE_FAILURE; //add another message in Thread func.


	//Release Turnstile mutex IMMEDIATELY after owning it & begin resource entrance regime 
	if (RELEASE_TURNSTILE_MUTEX_FAILED == ReleaseMutex(*(p_lock->p_h_turnstileMutex))) {
		printf("Error: Thread no. %lu failed to release Turnstile mutex to begin reading a resource with code: %d.\nExiting\n", GetCurrentThreadId(), GetLastError());
		return STATUS_CODE_FAILURE;
	}




	//Obtain Ownership of the Readers entrance\departure passage mutex that updates the current number of Readers reading the resource atomically...
	if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
		p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex,					//Mutex handle pointer
		TIMEOUT_BASE_CONSTANT_MS * p_lock->numberOfThreads,								//Timeout = #Threads milliSeconds
		READ_ACCESS																		//Reading bit, for printing appropriate failure message if needed
	))   return STATUS_CODE_FAILURE; //add another message in Thread func.
	
	//Adding '1' to the Readers currently reading the resource
	numberOfRealTimeReadersCounter++;

	//Vlidating if the entering Reader is the first reader of the following bunch of Readers (If there will be one)
	if(RESOURCE_IS_NOT_EMPTY == numberOfRealTimeReadersCounter)
		//Singalling OFF the semaphore that is responsible over entry of Writer\Readers to the resource..
		if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
			p_lock->p_h_vaccantResourceStatusSemphore,									//Semaphore handle pointer
			TIMEOUT_BASE_CONSTANT_MS * MAX_LENGTH_TASK * p_lock->numberOfThreads,		//Timeout = (Max task length)*#Threads milliSeconds
			READ_ACCESS																	//Reading bit, for printing appropriate failure message if needed
		))	return STATUS_CODE_FAILURE; //add another message in Thread func.

	//Before finishing executing the Read lock & letting the thread proceed to read its' desired contents from the resource,
	// we release the entrance\departure procedure mutex...
	if (RELEASE_TURNSTILE_MUTEX_FAILED == ReleaseMutex(*(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex))) {
		printf("Error: Thread no. %lu failed to release Turnstile mutex to begin reading a resource with code: %d.\nExiting\n", GetCurrentThreadId(), GetLastError());
		return STATUS_CODE_FAILURE;
	}




	//Read locking was successful..
	return STATUS_CODE_SUCCESS;
}

BOOL read_release(lock* p_lock)
{
	//Input integrity validation
	if (NULL == p_lock) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}

	//Begin resource departure regime....

	//Obtain Ownership of the Readers entrance\departure passage mutex that updates the current number of Readers reading the resource atomically...
	if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
		p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex,					//Mutex handle pointer
		TIMEOUT_BASE_CONSTANT_MS * p_lock->numberOfThreads,								//Timeout = #Threads milliSeconds
		READ_ACCESS																		//Reading bit, for printing appropriate failure message if needed
	))   return STATUS_CODE_FAILURE; //add another message in Thread func.

	//Substracting '1' to the Readers currently reading the resource
	numberOfRealTimeReadersCounter--;

	//Validating whther this exiting Reader is the last Reader in the bunch
	if (RESOURCE_IS_EMPTY == numberOfRealTimeReadersCounter)
		//Singalling ON the semaphore that is responsible over entry of Writer\Readers to the resource, if all readers left...
		if (RELEASE_EMPTY_RESOURCE_SEMAPHORE_FAILED == ReleaseSemaphore(
			*(p_lock->p_h_vaccantResourceStatusSemphore),							//Semaphore handle 
			1,																		//Rise the Semaphore count by 1 only!
			NULL																	//No need to validate the previous count
		)) {
			printf("Error: Thread no. %lu failed to signal the vacant-resource-Semaphore with code: %d.\nExiting\n", GetCurrentThreadId(), GetLastError());
			return STATUS_CODE_FAILURE;
		}

	//Before finishing executing the Read lock release & letting the thread proceed to further operations,
	// we release the entrance\departure procedure mutex...
	if (RELEASE_TURNSTILE_MUTEX_FAILED == ReleaseMutex(*(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex))) {
		printf("Error: Thread no. %lu failed to release Turnstile mutex to begin reading a resource with code: %d.\nExiting\n", GetCurrentThreadId(), GetLastError());
		return STATUS_CODE_FAILURE;
	}

	//Read lock release was successful..
	return STATUS_CODE_SUCCESS;
}




BOOL write_lock(lock* p_lock)
{
	//Input integrity validation
	if (NULL == p_lock) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}

	//Obtain Ownership of the Turnstile mutex that chooses between a new Reader to a new Writer...
	if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
		p_lock->p_h_turnstileMutex,														//Mutex handle pointer
		TIMEOUT_BASE_CONSTANT_MS * MAX_LENGTH_TASK * p_lock->numberOfThreads,			//Timeout = (Max task length)*#Threads milliSeconds
		READ_ACCESS-1																	//Writing bit, for printing appropriate failure message if needed
	))   return STATUS_CODE_FAILURE; //add another message in Thread func.


	/* ---------------------------------------------------------------------------------------------------------- */
    /* NOT ALL RESOURCES HAVE READERS (WRITERS ONLY RESOURCE) -> SO THEIR LOCK WILL ONLY HAVE THE TURNSTILE MUTEX */
    /* ---------------------------------------------------------------------------------------------------------- */
	if(NULL != p_lock->p_h_vaccantResourceStatusSemphore)
		//If Semaphore is signalling, the Writer may decrease its' count & enter the resource for a writing operation..
		if (STATUS_CODE_FAILURE == waitForSynchronousObjectToSignalOrUnlock(
			p_lock->p_h_vaccantResourceStatusSemphore,											//Semaphore handle pointer
			TIMEOUT_BASE_CONSTANT_MS * MAX_LENGTH_TASK *p_lock->numberOfThreads,				//Timeout = (Max task length)*#Threads milliSeconds * 10
			READ_ACCESS - 1																		//Writing bit, for printing appropriate failure message if needed
		)) {
			//Release Turnstile mutex in case of Readers not vacating the resource fast enough!!!!!! 
			if (RELEASE_TURNSTILE_MUTEX_FAILED == ReleaseMutex(*(p_lock->p_h_turnstileMutex))) {
				printf("Error: Thread no. %lu failed to release Turnstile mutex to finish writing to a resource with code: %d.\nExiting...\n", GetCurrentThreadId(), GetLastError());
				return STATUS_CODE_FAILURE;
			}
			return STATUS_CODE_FAILURE; //add another message in Thread func.
		}

	//Writer may now access the resource & perform a witing operation (Writing lock succeeded)
	return STATUS_CODE_SUCCESS;
}

BOOL write_release(lock* p_lock)
{
	//Input integrity validation
	if (NULL == p_lock) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}

	//Release Turnstile mutex & finish resource departure regime (Writing) 
	if (RELEASE_TURNSTILE_MUTEX_FAILED == ReleaseMutex(*(p_lock->p_h_turnstileMutex))) {
		printf("Error: Thread no. %lu failed to release Turnstile mutex to finish writing to a resource with code: %d.\nExiting...\n", GetCurrentThreadId(), GetLastError());
		return STATUS_CODE_FAILURE;
	}

	/* ---------------------------------------------------------------------------------------------------------- */
	/* NOT ALL RESOURCES HAVE READERS (WRITERS ONLY RESOURCE) -> SO THEIR LOCK WILL ONLY HAVE THE TURNSTILE MUTEX */
	/* ---------------------------------------------------------------------------------------------------------- */
	if (NULL != p_lock->p_h_vaccantResourceStatusSemphore)
		if (RELEASE_EMPTY_RESOURCE_SEMAPHORE_FAILED == ReleaseSemaphore(
			*(p_lock->p_h_vaccantResourceStatusSemphore),							//Semaphore handle 
			1,																		//Rise the Semaphore count by 1 only!
			NULL																	//No need to validate the previous count
		)) {
			printf("Error: Thread no. %lu failed to enable the vacant-resource-Semaphore to signal with code: %d.\nExiting...\n", GetCurrentThreadId(), GetLastError());
			return STATUS_CODE_FAILURE;
		}

	

	//Writing lock release succeeded
	return STATUS_CODE_SUCCESS;
}



BOOL DestroyLock(lock** p_p_lock)
{
	lock* p_lock = NULL;
	//Input integrity validation
	if (NULL == p_p_lock) {
		printf("Error: Bad input to function: %s\n", __func__);  return STATUS_CODE_FAILURE;
	}

	//For ease of access..
	p_lock = *p_p_lock;

	//Closing all the Synchronous Objects' handles
	printf("Beginning Lock destruction...\n");
	closeHandleProcedure(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
	closeHandleProcedure(p_lock->p_h_turnstileMutex);
	closeHandleProcedure(p_lock->p_h_vaccantResourceStatusSemphore);

	/*//Freeing the Handles' dynamically allocated memory
	free(p_lock->p_h_readersCountingAtEntryOrDepartureProceduresMutex);
	free(p_lock->p_h_turnstileMutex);
	free(p_lock->p_h_vaccantResourceStatusSemphore);
	*/

	//Freeing the lock struct
	free(p_lock);

	//Placing NULL to the pointer addressing the lock struct
	*p_p_lock = NULL;

	//Lock destruction was successful...
	return STATUS_CODE_SUCCESS;
}





//.................................................Static functions...............................................
static BOOL waitForSynchronousObjectToSignalOrUnlock(HANDLE* p_h_objectHandle, DWORD timeout, DWORD readOrWriteState)
{
	DWORD syncObjectWaitCode = 0;

	//Wait for the synchronous object to either signal or get unlocked for the thread's request to either read\write from\to the resource...
	// Resource == either the Priorities Tasks Queue or the Tasks file 
	syncObjectWaitCode = WaitForSingleObject(*p_h_objectHandle, timeout);

	//Validate the returning Wait code
	switch (syncObjectWaitCode)
	{
	case WAIT_TIMEOUT:	//Waiting Timeout reached
		if (READ_ACCESS == readOrWriteState) printf("Thread no. %lu reached timeout while waiting to be granted a reading access\n", GetCurrentThreadId());
		else printf("Thread no. %lu reached timeout while waiting to be granted a writing access\n", GetCurrentThreadId());
		return STATUS_CODE_FAILURE;

	case WAIT_OBJECT_0:	//Waiting ended with a positive response (signalling or unlocking)
		return STATUS_CODE_SUCCESS;

	default:			//Other failures... 
		if (READ_ACCESS == readOrWriteState) printf("Thread no. %lu didn't receive access for reading a resource with code 0x%x\n", GetCurrentThreadId(), syncObjectWaitCode);
		else printf("Thread no. %lu didn't receive access for reading a resource with code 0x%x\n", GetCurrentThreadId(), syncObjectWaitCode);
		printf("0x%x\n", syncObjectWaitCode);
		return STATUS_CODE_FAILURE;
	}
}