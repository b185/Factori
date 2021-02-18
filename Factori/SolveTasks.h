/* SolveTasks.h
--------------------------------------------------------
	Module Description - Header module for SolveTasks.c
--------------------------------------------------------
*/


#pragma once
#ifndef __SOLVE_TASKS_H__
#define __SOLVE_TASKS_H__


// Library includes -------------------------------------------------
#include <Windows.h>


// Projects includes ------------------------------------------------
#include "HardCodedData.h"
#include "MemoryHandling.h"
#include "FrameFileLines.h"
#include "Queue.h"
#include "TasksSolverThread.h"

//DELETE '&'

//Functions Declarations
/// <summary>
/// Description - This function receives the input Tasks list file and Tasks Priorities file, the number of threads the program should use for solving 
///		the tasks and the number of tasks. The function begins by "framing" the Tasks Priorities file's lines as byte offsets and lines sizes within a
///		"line"s nested list's elements (struct) that lies within a "file" struct. Later, the "file" struct, aka file's data' is used to construct the 
///		Tasks Priorities Queue. Afterwards, another "file" struct is created to describe the Tasks list file framing. Following that, the function
///		allocates memory for Handles (Threads, Mutexes, Semaphores), threads IDs and constructs the threads' input parameters object by arranging the
///		paramters (pointers to the resources and their locks) inside a "threadPackage" struct. Then the function creates all #Threads threads that begin
///		solving the tasks and writing their outputs to the Tasks file. Finally, after threads termination, the function assures all threads completed their
///		operation and exitted with the desired exit codes ('1'), and closes all remaining handles and freeing all remaining dynamically allocated memory.   
/// </summary>
/// <param name="char* p_tasksListFilePath - A pointer to the commandline argument (string) that represents the Tasks list file's path (absolute\relative)"></param>
/// <param name="char* p_tasksPrioritiesListFilePath - A pointer to the commandline argument (string) that represents the Tasks Priorities list file's path (absolute\relative)"></param>
/// <param name="int numberOfTasks - An integer that represents the number of tasks the program needs to solve"></param>
/// <param name="int numberOfThreads - An integer that represents the number of threads the program will use for the tasks solving procedure."></param>
/// <returns>A BOOL value representing the function's outcome (All tasks were solved and documented). Success (TRUE) or Failure (FALSE)</returns>
BOOL solveTasks(char* p_tasksListFilePath, char* p_tasksPrioritiesListFilePath, int numberOfTasks, int numberOfThreads);
	/*file* p_tasksPrioritiesFileData,*//*queue* p_tasksPrioritiesQueue, file* p_tasksFileData*/

/// <summary>
///  Description - This function is responsible on reading either a task's byte-offset-from-the-beginning of the Tasks list file (tasks' initial byte 
///      position in the Tasks list file) from the Tasks Priorities file    OR    a task's number by itself from the Tasks list file, and place
///		 the retrieved value in a predetermined address of an unsigned datatype variable defined outside this function.
///		 The function is given a pointer to a handle to the desired file, an initial byte position, number of bytes it should read from the
///		 file and a pointer to the variable that is meant to hold the retrieved value.
/// </summary>
/// <param name="HANDLE* p_h_fileHandle - A pointer to the Handle of a text file (either the Tasks list or Tasks Priorities list)"></param>
/// <param name="DWORD initialByteOffset - An unsigned integer represeting the starting read position (initial byte poisition-offset)"></param>
/// <param name="DWORD numberOfBytesToRead - An unsigned integer representing the number of bytes that should be read that represent either a task or a task's byte-offset"></param>
/// <param name="LPDWORD translatedNumber - A pointer to the variable that is meant to hold the read value"></param>
/// <returns>A BOOL value representing the function's outcome (The desired value was retrieved and placed in the predetermined location). Success (TRUE) or Failure (False)</returns>
BOOL fetchTaskByteOffsetByPriorityOrTaskItself(HANDLE* p_h_fileHandle, DWORD initialByteOffset, DWORD numberOfBytesToRead, LPDWORD translatedNumber);

#endif //__SOLVE_TASKS_H__
