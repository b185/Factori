/*
---------------------------------------------------------------------------------------------------------

	Project - "Factori" process for dicomposing numbers to their primal multipliers with multithreading

	Description - This is the main process that initiates the main thread that is meant to receive
		a path to a Tasks list txt file, Tasks Priorities list txt file, the number of threads it is
		supposed to create and the number of tasks it needs to solve & document.
		The process will evantually write all the tasks' solutions to the already existing Tasks-list 
		file (to the end of the file).
---------------------------------------------------------------------------------------------------------
*/

// Library includes -------------------------------------------------------------------------------------
#include <stdio.h>
#include <Windows.h>

// Projects includes ------------------------------------------------------------------------------------
//include every header
#include "FetchAndValidateCommandlineArguments.h"
#include "SolveTasks.h"


// Constants ----------------------------------------------------------------------------
static const BOOL STATUS_CODE_FAILURE = FALSE;
static const BOOL STATUS_CODE_SUCCESS = TRUE;


// Function Definition ------------------------------------------------------------------

int main(int argc, char* argv[]) {
	int numberOfTasks = 0, numberOfThreads = 0;
	//Validating the number of command line arguments
	if ((argc != 5) || (argv[1] == NULL) || (argv[2] == NULL) || (argv[3] == NULL) || (argv[4] == NULL)) {
		printf("Error: Incorrect number of arguments.\n");
		return 1;
	}


	/* --------------------------------------------------------------------------------------------------------------------------- */
	/*Validate that both the Tasks list text file & Tasks Priorities list text file paths are not null or '\0'			*/
	/*Read the number of tasks from string & validate that the number is positive							*/					   */
	/*Read the number of threads needed to be activated for the message processing & validate that the number is positive		*/
	/* --------------------------------------------------------------------------------------------------------------------------- */
	if (STATUS_CODE_FAILURE == fetchAndValidateCommandLineArguments(argv[1], argv[2],
									argv[3], &numberOfTasks,
									argv[4], &numberOfThreads)) return 1;


	

	

	
	/* --------------------------------------------------------------------------------------------------------------------------- */
	//The following function will perform all needed phases of the program process:												   */
	/* produce "file" struct describing the input files, then it will create a "queue" struct of the tasks' priorities.			   */
	/* Following that, it will create threads, that will, parallely, solve all the tasks & write these solutions to the end of the */
	/* Tasks file (documentation). Lastly, after threads termination, all the wait & exit codes of the threads will be validated   */
	/* and the operation final status code will be outputted																	   */		
	/* --------------------------------------------------------------------------------------------------------------------------- */
	if (STATUS_CODE_FAILURE == solveTasks(argv[1], argv[2], numberOfTasks, numberOfThreads)) {
		printf("Error: Failed to solve all tasks.\n");
		return 1;
	}

	
	


	
	printf("ALL TASKS WERE SOLVED CORRECTLY & SUCCESSFULY!!!\n\n\n\n\n\n");
	return 0;
}
