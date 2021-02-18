/* FetchCommandlineArguments.c
---------------------------------------------------------------------------------
	Module Description - This module contains functions meant for processing the 
		tasks solving procedure properties from the commandline arguments, and 
		place them in variables defined with the appropriate data types. 
		e.g. number of tasks, number of threads with which the tasks will be
		solved. It also validated the integrity of the inputs e.g. file paths.
---------------------------------------------------------------------------------
*/

// Library includes -------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes -----------------------------------------------------------
#include "FetchAndValidateCommandlineArguments.h"

// Constants
static const BOOL STATUS_CODE_FAILURE = FALSE;
static const BOOL STATUS_CODE_SUCCESS = TRUE;


// Functions declerations ------------------------------------------------------

/// <summary>
/// Description - This function receives a command line argument of an integer number string
/// and translates it to an intger, and places this value into a variable address, that is also received as input.
/// </summary>
/// <param name="char* p_commandLineString - A pointer to the commandline argument (string) that represents an integer number"></param>
/// <param name="int* p_argumentNumberAddress - A pointer to the number's integer variable's address"></param>
/// <returns>A BOOL value representing the function's outcome (conversion). Success (TRUE) or Failure (FALSE)</returns>
static BOOL fetchArgumentNumber(char* p_commandLineString, int* p_argumentNumberAddress);



// Functions definitions -------------------------------------------------------

BOOL fetchAndValidateCommandLineArguments(char* p_tasksListFilePath, char* p_tasksPrioritiesListFilePath,
	char* p_commandLineNumberOfTasksString, int* p_numberOfTasksAddress,
	char* p_commandLineNumberOfThreadsString, int* p_numberOfThreadsAddress)
{
	//Input integrity validation
	if ((NULL == p_tasksListFilePath) || (NULL == p_tasksPrioritiesListFilePath) || (NULL == p_commandLineNumberOfTasksString) || (NULL == p_numberOfTasksAddress) \
		|| (NULL == p_commandLineNumberOfThreadsString) || (NULL == p_numberOfThreadsAddress)) {
		printf("Error: Bad inputs to function: %s\n", __func__); return STATUS_CODE_FAILURE;
	}
	

	//Validating the intergrity of the input files paths
	if (('\0' == *p_tasksListFilePath) ||('\n' == *p_tasksListFilePath)) {
		printf("Error: Failed to recieve a vaild path of the Tasks list text file.\n"); return STATUS_CODE_FAILURE;
	}
	if (('\0' == *p_tasksPrioritiesListFilePath) || ('\n' == *p_tasksPrioritiesListFilePath)) {
		printf("Error: Failed to recieve a vaild path of the Tasks Priorities list text file.\n"); return STATUS_CODE_FAILURE;
	}



	//Fetching the Number of Tasks the program will need to solve
	if (STATUS_CODE_SUCCESS != fetchArgumentNumber(p_commandLineNumberOfTasksString, p_numberOfTasksAddress)) {
		printf("Error: Failed to read the number of tasks value from commandline.\n");
		return STATUS_CODE_FAILURE;
	}
	//Validating the Number of Tasks has a positive value
	if (*p_numberOfTasksAddress <= 0) {
		printf("Error: Failed to receive a valid number of tasks to operate.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}
	


	//Fetching the Number of Threads the process will need to activate when processing the given message
	if (STATUS_CODE_SUCCESS != fetchArgumentNumber(p_commandLineNumberOfThreadsString, p_numberOfThreadsAddress)) {
		printf("Error: Failed to read the number of threads value from commandline.\n");
		return STATUS_CODE_FAILURE;
	}
	//Validating the number of threads is a positive number
	if (*p_numberOfThreadsAddress <= 0) {
		printf("Error: Failed to receive a number of threads which is positive.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}



	//Retrieving arguments values was successful...
	return STATUS_CODE_SUCCESS;
}



//......................................Static functions..........................................

static BOOL fetchArgumentNumber(char* p_commandLineString, int* p_argumentNumberAddress)
{
	errno_t retVal;
	//Asserts
	assert(p_commandLineString != NULL);
	assert(p_argumentNumberAddress != NULL);

	//Scanning the string-number from the commandline argument in to an integer address
	if ((retVal = sscanf_s(p_commandLineString, "%d", p_argumentNumberAddress)) == EOF) {
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Key was retrieved successfuly and is legitimate
	return STATUS_CODE_SUCCESS;
}