/* TasksSolverThread.h
---------------------------------------------------------------
	Module Description - Header module for TasksSolverThread.c
---------------------------------------------------------------
*/


#pragma once
#ifndef __TASKS_SOLVER_THREAD_H__
#define __TASKS_SOLVER_THREAD_H__


// Library includes -------------------------------------------------
#include <Windows.h>


// Projects includes ------------------------------------------------
#include "HardCodedData.h"
#include "MemoryHandling.h"
#include "Queue.h"
#include "Lock.h"
#include "ReceivePrimeFactorizedListString.h"
#include "SolveTasks.h"



//Functions Declarations
/// <summary>
/// Description - This function is the Thread routine. It receives a "threadPackage" data as input, which will contain the all the resources, and their locks,
///		needed to assist it execute the following routine, until there are no more tasks needed to be solved (Empty Tasks Priorities queue):
///		1) write_lock the queue resource  
///	    2) draw the most prioritized task offset from resource1-queue (top of the queue)   
///		3) release writing lock (resource1)
///	    4) fetch number of bytes in the received offset in Tasks file (from resouce2-Tasks file data struct - a Readers ONLY struct - no need for locking)
///		5) read_lock resource3-Tasks file  
///		6) fetch task's value   
///		7) release read lock    
///		8) solve task...   
///		9) write_lock resource3   
///		10) write to file the solution
///		11) release writing lock  
///		12) cycle back as long as the queue has values in it (will be check after relocking) 
///		Finally, when the queue is empty, the thread will exit with Success code.
/// </summary>
/// <param name="LPVOID lpParam - A void pointer to the shared-with-all-threads 'threadPackage' data struct"></param>
/// <returns>A BOOL value representing the function's outcome (Thread has completed solving and printing to Tasks file all the tasks it took from the Tasks Priorities Queue). Success (TRUE) or Failure (FALSE)</returns>
BOOL WINAPI tasksSolverThread(LPVOID lpParam);


#endif //__TASKS_SOLVER_THREAD_H__
