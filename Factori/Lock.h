/* Lock.h
--------------------------------------------------
	Module Description - header module for Lock.c
--------------------------------------------------
*/


#pragma once
#ifndef __LOCK_H__
#define __LOCK_H__


// Library includes --------------------------------------------
#include <Windows.h>


// Projects includes -------------------------------------------
#include "HardCodedData.h"
#include "MemoryHandling.h"



//Functions Declarations

/// <summary>
///	 Description - This function allocates memory (on the Heap) for a "lock" struct and returns a pointer to it (calloc)
/// </summary>
/// <returns>A pointer to an initiated 'lock' struct if successful, or NULL if failed.</returns>
lock* InitializeLock();
/// <summary>
///  Description - This function describes the read lock request routine, and initially consists of owning the Turnstile Mutex and releasing it.
///		Following that, an ownership request will be made toward the Readers Mutex, and if granted the Real-Time-Readers-currently-accessing-the-resource
///		count will be incremented. In the case of an vacant resource (no thread is accessing the resource no matter the type of access), then the
///		vacant-resource Semaphore is signalled OFF by the read-lock requesting thread, thus, making the resource access currently exclusive for Readers.
///		Finally, the Readers Mutex is unlocked by the thread (thus making the current-Readers count increment procedure uniteruptted by any other Reader thread)
/// </summary>
/// <param name="lock* p_lock - A pointer to a resource 'lock' struct that contains the resource synchronous objects and the lock's timeout components"></param>
/// <returns>A BOOL value representing the function's outcome (Read lock status). Success (TRUE) or Failure (False)</returns>
BOOL read_lock(lock* p_lock);
/// <summary>
///  Description - This function describes the read lock release request routine, and consists an ownership request of the Readers Mutex in order to decrement 
///		the Real-Time-Readers-currently-accessing-the-resource count, and in the case of a now-becoming-a-vacant-resource, the vacant resource Semaphore will be signalled ON
///		by the lock releasing thread. Then the Readers Mutex is unlocked by the thread (thus making the current-Readers count decrement procedure uniteruptted by any other Reader thread).
/// </summary>
/// <param name="lock* p_lock - A pointer to a resource 'lock' struct that contains the resource synchronous objects and the lock's timeout components"></param>
/// <returns>A BOOL value representing the function's outcome (Read lock release status). Success (TRUE) or Failure (False)</returns>
BOOL read_release(lock* p_lock);
/// <summary>
///  Description - This function describes the write lock request routine, and initially consists of owning the Turnstile Mutex. If the ownership succeeded then
///		the thread(Writer) will wait until the Readers Semaphore is signalled ON (and by so, it will be know that all Readers exitted the resource, and the resource
///		is vacant for accessing by a Writing thread), then the thread will signal it OFF for exclusivness. The capturing of the Turnstile mutex assists in preventing
///		a Starvation condition.
/// </summary>
/// <param name="lock* p_lock - A pointer to a resource 'lock' struct that contains the resource synchronous objects and the lock's timeout components"></param>
/// <returns>A BOOL value representing the function's outcome (Write lock status). Success (TRUE) or Failure (False)</returns>
BOOL write_lock(lock* p_lock);
/// <summary>
///  Description - This function describes the write lock release request routine, and initially consists of releasing ownership of the Turnstile Mutex. 
///		Then the write locking thread will singal ON the Readers Semaphore and will alert that there is no more a Writer in the resource.
/// </summary>
/// <param name="lock* p_lock - A pointer to a resource 'lock' struct that contains the resource synchronous objects and the lock's timeout components"></param>
/// <returns>A BOOL value representing the function's outcome (Write lock release status). Success (TRUE) or Failure (False)</returns>
BOOL write_release(lock* p_lock);
/// <summary>
///  Description - This function destroys the lock by releasing any memory allocated to the "lock" struct and closing any open handles to
///		the Synchronous Objects the lock points at.
///		It also sets the lock's pointer to point at NULL address, and returns a final status code of the operation.
/// </summary>
/// <param name="lock** p_p_lock - A pointer to a pointer that points at a lock struct"></param>
/// <returns>A BOOL value representing the function's outcome (Queue mem. is released and its' pointer points at NULL). Success (TRUE) or Failure (False)</returns>
BOOL DestroyLock(lock** p_p_lock);


#endif //__LOCK_H__