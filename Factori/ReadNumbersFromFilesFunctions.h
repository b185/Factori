/* ReadNumbersFromFilesFunctions.h
----------------------------------------------------------------------------------
	Module Description - header module for ReadNumbersFromFilesFunctions.c
----------------------------------------------------------------------------------
*/


#pragma once
#ifndef __FETCH_AND_VALIDATE_COMMANDLINE_ARGUMENTS_H__
#define __FETCH_AND_VALIDATE_COMMANDLINE_ARGUMENTS_H__


// Library includes -------------------------------------------------------
#include <Windows.h>



// Projects includes ------------------------------------------------------
#include "HardCodedData.h"


//Functions Declarations

/// <summary>
/// Description - This function receives the command line arguments and translates
///		the strings representing them to values with the desired data types, and places
///		these values into variables addresses that are also received as input.
///		It also validates if the files paths are legal or not.
///		Evantually, the function chains back to "main" whether the inputs are legal or not.
/// </summary>
/// <param name="char* p_tasksListFilePath - A pointer to the Tasks list file path string (null terminated characters array)"></param>
/// <param name="char* p_tasksPrioritiesListFilePath - A pointer to the Tasks Priorities list file path string (null terminated characters array)"></param>
/// <param name="char* p_commandLineNumberOfTasksString - A pointer to the commandline argument (string) that represents the number of tasks the program will need to solve"></param>
/// <param name=" int* p_numberOfTasksAddress - A pointer to the number's integer variable's address in main"></param>
/// <param name="char* p_commandLineNumberOfThreadsString - A pointer to the commandline argument (string) that represents the number of threads the program will use to solve the tasks it receives"></param>
/// <param name=" int* p_numberOfThreadsAddress - A pointer to the number's integer variable's address in main"></param>
/// <returns>A BOOL value representing the function's outcome. Success (TRUE) or Failure (FALSE)</returns>
BOOL fetchAndValidateCommandLineArguments(char* p_tasksListFilePath, char* p_tasksPrioritiesListFilePath,
	char* p_commandLineNumberOfTasksString, int* p_numberOfTasksAddress,
	char* p_commandLineNumberOfThreadsString, char* p_numberOfThreadsAddress);

#endif //__FETCH_AND_VALIDATE_COMMANDLINE_ARGUMENTS_H__
