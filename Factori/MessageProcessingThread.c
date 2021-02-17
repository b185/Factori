/* MessageProcessingThread.c
-----------------------------------------------------------------------------------------
	Module Description - This module contains the thread routine that will be executed
		by every thread created by the main thread (the process). It will also contain
		additional functions meant to assist the thread to read from an input
		file the thread's designated section in bytes, decrypting or encrypting this
		section of the message and writing the processed section to the output file.
-----------------------------------------------------------------------------------------
*/

// Library includes ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes --------------------------------------------------------------------
#include "MessageProcessingThread.h"

// Constants
static const BOOL STATUS_CODE_FAILURE = FALSE;
static const BOOL STATUS_CODE_SUCCESS = TRUE;

static const BOOL STATUS_FILE_READING_FAILED = 0;
static const BOOL STATUS_FILE_WRITING_FAILED = 0;


static const int CHARACTER_IS_DIGIT = 1;
static const int CHARACTER_IS_UPPER_CASE_LETTER = 2;
static const int CHARACTER_IS_LOWER_CASE_LETTER = 3;
static const int CHARACTER_IS_INVALID = -1;


static const LONG DUMMY_CONSTANTS_FOR_SEPHAMORE = 5;


///SEARCH FOR 'TAKE NOTE'


// Global variables
	//Encryption\Decryption key
int encDecKey = 0;


// Functions declerations ---------------------------------------------------------------
/* ----openFileForReading(.)----
	Description - This function receives a file path (Relative\Absolute), and uses CreateFile of windows API
		to open the file. It sets the file Handle to GENERIC_READ mode for reading the input message, and
		sharing mode to FILE_SHARE_READ so when the threads run concurrently they won't interupt one another.
		Also the creation disposition is set to OPEN_EXISTING so it is possible to read the existing messsage.
		The function returns the Handle to the input file (CreateFile).

	Parameters -
		@param char* p_filePath - A pointer to a null-terminated-character-array(string) representing a file path

	Returns - A HANDLE type object to the file in the path (After attempting opening it)
*/
static HANDLE openFileForReading(char* p_filePath);
/* ----openFileForWriting(.)----
	Description - This function receives a file path (Relative\Absolute), and uses CreateFile of windows API
		to open the file. It sets the file Handle to GENERIC_WRITE mode for writing the processed message, and
		sharing mode to FILE_SHARE_WRITE so when the threads run concurrently they won't interupt one another to write
		to their respective sections in the message. Also the creation disposition is set to OPEN_EXISTING so it is 
		possible to read the existing messsage. The function returns the Handle to the output file (CreateFile). 

	Parameters -
		@param char* p_filePath - A pointer to a null-terminated-character-array(string) representing a file path

	Returns - A HANDLE type object to the file in the path (After attempting opening it)
*/
static HANDLE openFileForWriting(char* p_filePath);
/* ----fetchSection(.)----
	Description - This function receives a Handle to the input message file & a pointer to
		a section data struct, and with it, the function reads a string containing all
		characters in the thread's section in the message. The function returns a pointer
		to that string.

	Parameters -
		@param HANDLE h_msgFileHandle - A Handle to the input message file.
		@param section* p_sectionData - A pointer to a "section" data struct that 
			describes the thread's section initial byte offset & number of bytes in the section.

	Returns - A pointer to a string containing the section's characters (including newline chars) if successful, or NULL if failed.
*/
static LPTSTR fetchSection(HANDLE h_msgFileHandle, section* p_sectionData);
/* ----checkCharacter(.)----
	Description - This function receives a character value and simply returns an integer value
		indicating if the character is a lower case letter, upper case letter, a digit
		or anything elses.

	Parameters -
		@param TCHAR character - A character.
		
	Returns - An integer. Indication of either lower case letter(3), upper case letter(2), digit(1) or else(-1).
*/
static int checkCharacter(TCHAR character);
/* ----characterConversion(.)----
	Description - This function receives a character value and simply returns the converted
		character, either decrypted or encrypted, according to its' status, using the Caesar code,
		when the key is already	set to either of those modes (by multiplying it by minus 1 or not, in initialProcessing()).
		The encryption\decryption key is the global variable - encDecKey.

	Parameters -
		@param TCHAR character - A character from the thread's section in the message.
		@param int characterStatus - An integer indicating whether the character is upper/lower case letter or a digit.


	Returns - A TCHAR character - The converted character (either decrypted or encrypted).
*/
static TCHAR characterConversion(TCHAR character, int characterStatus);
/* ----processSection(.)----
	Description - This function receives a pointer to a TCHAR string the contains all the characters in
		the thread's section, and slowly iterating through all of them, one at a time, and converting them
		with using the encryption\decryption key (global variable - encDecKey). The function returns a pointer
		to the same address, after this string was converted.

	Parameters -
		@param LPTSTR p_sectionString - A pointer to TCHAR string containing the section's characters.

	Returns - A TCHAR pointer to the converted string (processed/decrypted-encrypted).
*/
static LPTSTR processSection(LPTSTR p_sectionString);
/* ----printToOutputFile(.)----
	Description - This function receives a Handle to the output message file, a pointer to
		a section data struct & a pointer to the processed section of bytes, and with them, the function writes
		the processed (converted, encrypted\decrypted) section arranged as a string, to the output message file. 
		The function returns a value representing whether the function succeeded to write the data to the file or not.

	Parameters -
		@param HANDLE h_outputFileHandle - A Handle to the output message file.
		@param section* p_sectionData - A pointer to a "section" data struct that
			describes the thread's section initial byte offset & number of bytes in the section.
		@param LPTSTR p_sectionString - A pointer to TCHAR string containing the section's processed characters.

	Returns - A BOOL value representing the function's outcome (Processed thread's section written). Success (TRUE) or Failure (FALSE).
*/
static BOOL printToOutputFile(HANDLE h_outputFileHandle, section* p_sectionData, char* p_sectionString);

//BONUS
/* ----waitSemaphoreToShoot(.)----
	Description - Following the creation of the already existing semaphore created by the main thread and initialized
		to zero, this function will activate WaitForSingleObject with a time cap of INFINITE, so the current thread
		(And the rest of the threads) will no resume (start) operation until the main thread releases the semaphore
		with ReleaseSemaphore by the number of threads. When that happens, this thread (and all threads) will receive the
		signal from the semaphore and resume operation, in a way that will make all the threads begin operation TOGETHER.

	Parameters - 
		@param HANDLE h_semaphore - A Handle to the already existing semaphore

	Returns - A BOOL value representing the function's outcome (Thread was signal by the operation of releasing the semaphore).
		Success (TRUE) or Failure (FALSE)
*/
static BOOL waitForSemaphoreToShoot(HANDLE h_semaphore);




// Functions definitions ----------------------------------------------------------------

BOOL WINAPI messageProcessingThread(LPVOID lpParam) {

	section* p_params = NULL;

	// Variables
	HANDLE h_inputFileHandle = NULL, h_outputFileHandle = NULL, h_semaphore = NULL;
	LPTSTR p_sectionString = NULL;


	//Check whether lpParam is NULL - Input integrity validation
	if (NULL == lpParam) return EMPTY_THREAD_PARAMETERS;

	//Parameters input conversion from void pointer to section struct pointer by explicit type casting
	p_params = (section*)lpParam;


	//Creating a handle for the-already-existing semaphore in order to wait for all threads to be created
	// This is done by creating the semaphore with its' name that was passed down through the thread's paramters
	if (NULL == (h_semaphore = CreateSemaphore(
		NULL,								/*  default security attributes - no need that the thread can be inherited by child processes */
		DUMMY_CONSTANTS_FOR_SEPHAMORE,		/*  initial count - will be ignored when fetching the existing sephamore handle */
		DUMMY_CONSTANTS_FOR_SEPHAMORE + 1,	/*  maximum count - will be ignored when fetching the existing sephamore handle */
		p_params->p_semaphoreName			/*  semaphore name - the name that was created in the main thread and will now assist us to receive a handle for this sephamore and await its signal*/
	))) {
		printf("Fatal error: Failed to recreate simultaneous-thread-activation sephamore from within a child-thread, with code: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}

	//Awaiting the sephamore's signal for union threads activation...
	if (STATUS_CODE_FAILURE == waitForSemaphoreToShoot(h_semaphore)) { closeHandleProcedure(h_semaphore); return STATUS_CODE_FAILURE; }



	//Special case where the thread wasn't given a section to decrypt\encrypt so it does nothing and terminates....
	if (p_params->numberOfBytesInSection == 0)   return STATUS_CODE_SUCCESS;


	//Insert time stamp





	//Updating global variable for ease of access (later at step3 compare with flag, and insert minus accordingly)
	encDecKey = p_params->encryptionDecryptionKey;

	//Opening a handle to the input file for the current thread 
	if (INVALID_HANDLE_VALUE == (h_inputFileHandle = openFileForReading(p_params->p_inputFilePath))) {
		printf("Error: Failed to open message file (input) from thread, with code: %d.\n", GetLastError());
		printf("At file: %s\n at line number: %d\n at function: %s\n", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Fetching the thread's section contructed as character string
	if (NULL == (p_sectionString = fetchSection(h_inputFileHandle, p_params))) {
		closeHandleProcedure(h_inputFileHandle);
		return STATUS_CODE_FAILURE;
	}
	//Closing the input message file handle opened by the thread, because the entire thread's section of the message was fetched
	closeHandleProcedure(h_inputFileHandle);


	//Process (Either encrypt or decrypt) the thread's section set as character string
	p_sectionString = processSection(p_sectionString);


	//Opening a handle to the output file for the current thread 
	if (INVALID_HANDLE_VALUE == (h_outputFileHandle = openFileForWriting(p_params->p_outputFilePath))) {
		printf("Error: Failed to open message file (output) from thread, with code: %d.\n", GetLastError());
		printf("At file: %s\n at line number: %d\n at function: %s\n", __FILE__, __LINE__, __func__);
		free(p_sectionString);
		return STATUS_CODE_FAILURE;
	}

	//Print the processed section (Either decrypted or encrypted) to the output file in the same byte addresses (positions\offsets)
	if (STATUS_CODE_FAILURE == (printToOutputFile(h_outputFileHandle, p_params, p_sectionString))) {
		closeHandleProcedure(h_outputFileHandle);
		free(p_sectionString);
		return STATUS_CODE_FAILURE;
	}

	//Closing output message file handle opened by the thread
	closeHandleProcedure(h_outputFileHandle);
	//Freeing the thread's message section buffer
	free(p_sectionString);




	//Insert time stamp




	//Thread successfuly written its' processed (Encrypted\Decrypted) section of the message to the output message file...
	return STATUS_CODE_SUCCESS;
}


//......................................Static functions..........................................

static HANDLE openFileForReading(char* p_filePath) 
{
	HANDLE h_fileHandle = NULL;
	//Assert
	assert(NULL != p_filePath);

	//Open file for reading
	h_fileHandle = CreateFile(
		p_filePath,				// Const null - terminated string describing the file's path
		GENERIC_READ,			// Desired Access is set to Reading mode 
		FILE_SHARE_READ,		// Share Mode:Here various threads may read from the input file (input message)
		NULL,					// No Security Attributes
		OPEN_EXISTING,			// The message is an existing file. If it doesn't exist we shouldn't open anything
		FILE_ATTRIBUTE_NORMAL,	// General reading in files 
		NULL					// No Template
	);
	//Returning the created handle to file
	return h_fileHandle;
}

static HANDLE openFileForWriting(char* p_filePath) //consider moving to different module
{
	HANDLE h_fileHandle = NULL;
	//Assert
	assert(NULL != p_filePath);

	//Open file for writing
	h_fileHandle = CreateFile(
		p_filePath,				// Const null - terminated string describing the file's path
		GENERIC_WRITE,			// Desired Access is set to Writing mode 
		FILE_SHARE_WRITE,		// Share Mode:Here various threads may write to the output file (output message)
		NULL,					// No Security Attributes
		OPEN_EXISTING,			// The message is an existing file. If it doesn't exist we shouldn't open anything
		FILE_ATTRIBUTE_NORMAL,	// General writing to files 
		NULL					// No Template
	);
	//Returning the created handle to file
	return h_fileHandle;
}



//Reading from file
static LPTSTR fetchSection(HANDLE h_msgFileHandle, section* p_sectionData)
{
	LPTSTR p_sectionString = NULL;
	DWORD retValSet = 0, numberOfBytesRead = 0;
	BOOL retValRead = FALSE;
	//Asserts
	assert(h_msgFileHandle != NULL);
	assert(p_sectionData != NULL);

	//Firstly, setting the file Handle to the initial byte position of the line
	// IMPORTANT AND NEEDS TO BE VERIFIED: This will become an implicit byte offset positioning for ReadFile
	//		and it may be less efficient for multi-threading operation because it is non-atomic, while 
	//		using an OVERLAPPED struct might turn ReadFile atomic!!!
	retValSet = SetFilePointer(
		h_msgFileHandle,									//Input file Handle 
		p_sectionData->initialBytePositionForProcessing,	//Initial byte position set to be the begining of the line
		NULL,												//Assuming there is less than 2^32 bytes in the input file -recheck
		FILE_BEGIN											//Starting byte count is set to the start of the file (byte=0)
	);
	//Validate Handle pointing succeeded...
	if (INVALID_SET_FILE_POINTER == retValSet) {
		//Initial byte position of the line wasn't found
		printf("Error: Failed to reset the file Handle pointer position for reading, with code: %d.\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}
	//Secondly, Memory allocation for the string buffer containing the line's characters
	if ((p_sectionString = (LPTSTR)calloc(sizeof(TCHAR), p_sectionData->numberOfBytesInSection + 1)) == NULL) {
		printf("Error: Failed to allocate memory for the section's string buffer.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}
	//Thirdly, reading from the file Handle the size of the line in bytes
	retValRead = ReadFile(
		h_msgFileHandle,							//Input file Handle after being set at the desired byte-positions
		p_sectionString,							//Section string buffer pointer
		p_sectionData->numberOfBytesInSection,		//Total number of bytes, needed to be read, of the thread's section in the message
		&numberOfBytesRead,							//Pointer to the total number of bytes fetched 
		NULL										//Overlapped - off
	);
	if (STATUS_FILE_READING_FAILED == retValRead) {
		//Failed to read the needed memory from the file
		printf("Error: Failed to read from the file Handle. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		free(p_sectionString);
		return NULL;
	}

	//Returning the section string buffer...
	return p_sectionString;
}



//Encrypting\Decrypting functions
static int checkCharacter(TCHAR character) //TCHAR* == LPTSTR	/// Checks whether a character is leagal or not ///
{
	assert(character != '\0');
	if (character >= '0' && character <= '9')
		return CHARACTER_IS_DIGIT;
	else if (character >= 'A' && character <= 'Z')
		return CHARACTER_IS_UPPER_CASE_LETTER;
	else if (character >= 'a' && character <= 'z')
		return CHARACTER_IS_LOWER_CASE_LETTER;
	return CHARACTER_IS_INVALID;
}

static TCHAR characterConversion(TCHAR character, int characterStatus)
{
	char negativeCharHandling = '\0';
	assert(character != '\0');
	assert((characterStatus >= 1) && (characterStatus <= 3));
	switch (characterStatus)
	{
		//HAD AN ERROR HERE MINUTES BEFORE SUBMISSION - TAKE NOTE
	case 1:
		//'0' + (digit-'0'-key)%10
		negativeCharHandling = /*(TCHAR)'0' +*/ (character - (TCHAR)'0' - encDecKey) % 10;
		if (negativeCharHandling < 0)
			return (TCHAR)(negativeCharHandling + '0' + 10);
		return (TCHAR)'0' + (character - (TCHAR)'0' - encDecKey) % 10;
	case 2:
		//'A' + (digit-'A'-key)%26
		negativeCharHandling = /*(TCHAR)'0' +*/ (character - (TCHAR)'A' - encDecKey) % 26;
		if (negativeCharHandling < 0)
			return (TCHAR)(negativeCharHandling + 'A' + 26);
		return (TCHAR)'A' + (26 + character - (TCHAR)'A' - encDecKey) % 26;
	case 3:
		//'a' + (digit-'a'-key)%26
		negativeCharHandling = /*(TCHAR)'0' +*/ (character - (TCHAR)'a' - encDecKey) % 26;
		if (negativeCharHandling < 0)
			return (TCHAR)(negativeCharHandling + 'a' + 26);
		return (TCHAR)'a' + (26 + character - (TCHAR)'a' - encDecKey) % 26;
	}
	return character; //false move but it will prevent a Warning from popping
}

static LPTSTR processSection(LPTSTR p_sectionString) //consider turning it to a void function
{
	DWORD index = 0;
	int characterStatus = 0;
	//Assert
	assert(p_sectionString != NULL);
	//Iterating through the entire characters in the section and process each accordingly (Encrypt/Decrypt)
	//	Checking whether end-of-section was reached. If it was reached, we can simply stop analyzing
	while (*(p_sectionString + index) != '\0') {
		//Firstly, identify the character
		characterStatus = checkCharacter(*(p_sectionString + index));
		//Secondly, validate whether the character is upper\lower\digit or none(-1)
		if (characterStatus != CHARACTER_IS_INVALID)
			//Thirdly, alter the digit according to the processing operation -encrypt\decrypt
			*(p_sectionString + index) = characterConversion(*(p_sectionString + index), characterStatus);
		//Lastly, advance to the following character in the line
		index++;
	}

	//Returning a pointer to the updated string line
	return p_sectionString;
}


//Writing to file
static BOOL printToOutputFile(HANDLE h_outputFileHandle, section* p_sectionData, char* p_sectionString)
{
	DWORD retValSet = 0, numberOfBytesWritten = 0;
	BOOL retValWrite = FALSE;
	//Asserts
	assert(h_outputFileHandle != NULL);
	assert(p_sectionData != NULL);
	assert(p_sectionString != NULL);
	//Set the Handle to point at the byte position from which the print to the file should start (and it is identical to the input message)
	// IMPORTANT AND NEEDS TO BE VERIFIED: This will become an implicit byte offset positioning for WriteFile
	//		and it may be less efficient for multi-threading operation because it is non-atomic, while 
	//		using an OVERLAPPED struct might turn WriteFile atomic!!!

	// FURTHERMORE: at this phase, when working with a single thread(main) that prints all of the lines, the handle will
	// simply remain where it stopped writing..(i think)
	retValSet = SetFilePointer(
		h_outputFileHandle,									//Output file Handle 
		p_sectionData->initialBytePositionForProcessing,	//Initial byte position set to be the begining of the section
		NULL,												//Assuming there is less than 2^32 bytes in the input file -recheck
		FILE_BEGIN											//Starting byte count is set to the start of the file (byte=0)
	);
	//Validate Handle pointing succeeded...
	if (INVALID_SET_FILE_POINTER == retValSet) {
		//Initial byte position of the line wasn't found
		printf("Error: Failed to reset the file Handle pointer position for printing, with code: %d.\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Writing to output file at the desired position
	retValWrite = WriteFile(
		h_outputFileHandle,							//Output file Handle after being set at the desired byte-position 
		p_sectionString,							//Section string buffer pointer
		p_sectionData->numberOfBytesInSection,		//Total number of bytes, needed to be written, of the thread's section in the message
		&numberOfBytesWritten,						//Pointer to the total number of bytes written 
		NULL										//Overlapped - off
	);
	if (STATUS_FILE_WRITING_FAILED == retValWrite) {
		//Failed to write the needed memory from the file
		printf("Error: Failed to write to the file Handle. Exited with code: %d\n", GetLastError());
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return STATUS_CODE_FAILURE;
	}

	//Writing to output file was successful...
	return STATUS_CODE_SUCCESS;
}



//BONUS step ADDITION
static BOOL waitForSemaphoreToShoot(HANDLE h_semaphore)
{
	DWORD semaphoreWaitCode = 0;
	assert(NULL != h_semaphore);

	//Wait for sephamore to SHOOT TO START THE RACE!!!!! 
	semaphoreWaitCode = WaitForSingleObject(h_semaphore, INFINITE);
	//Checking the waiting code
	switch (semaphoreWaitCode)
	{
	case WAIT_OBJECT_0:
		printf("Semaphore signaled\n");
		closeHandleProcedure(h_semaphore);
		return STATUS_CODE_SUCCESS;
	case WAIT_TIMEOUT:
		printf("In thread, WAIT_TIMEOUT\n");
		return STATUS_CODE_FAILURE;
	default:
		printf("0x%x\n", semaphoreWaitCode);
		return STATUS_CODE_FAILURE;
	}
}
