/* FrameFileLines.c
-----------------------------------------------------------------------------------------
	Module Description - This module contains functions meant for an initial file 
		reading phase. This initial complete reading of the file will enable the 
		construction of a data structure that will possess information concerning the 
		entire structure of the file e.g. initial byte position(offset) of every line, 
		number of bytes every line is comprised of etc.
-----------------------------------------------------------------------------------------
*/

// Library includes ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes --------------------------------------------------------------------
#include "FrameFileLines.h"



// Constants
static const BOOL STATUS_CODE_FAILURE = FALSE;
static const BOOL STATUS_CODE_SUCCESS = TRUE;

static const BOOL STATUS_FILE_READING_SUCCESS = TRUE;

static const DWORD PREVENT_SHARING = 0;

static const DWORD SINGLE_OBJECT = 1;

static const DWORD END_OF_FILE = 0;
static const BOOL  EMPTY_FILE = 5;


// Functions declerations ---------------------------------------------------------------
/// <summary>
///	 Description - This function allocates memory for a "file" struct in the heap, and initiates its' values (calloc)
/// </summary>
/// <returns>A pointer to an initiated "file" struct if successful, or NULL if failed.</returns>
static file* fileStructMemoryAllocation();
/// <summary>
///  Description - This function receives a file path (Relative\Absolute), and uses CreateFile of windows API
///		to open the file. It sets the file Handle to GENERIC_READ mode for reading the input file's contents, and
///		sharing prevention mode(because only the main thread is supposed to read it).
///		The function returns the Handle to the input file(CreateFile)
/// </summary>
/// <param name="char* p_filePath - A pointer to a null-terminated-character-array(string) representing a file path"></param>
/// <returns>A HANDLE type object to the file in the path (After attempting opening it)</returns>
static HANDLE* openFileForInitialRead(char* p_filePath);
/// <summary>
///  Description - This function, after reaching the end of the line, indicated by a newline character,
///		updates a file struct by adding another line to its' count, and updates the current line struct with
///		the total number of bytes in this line. It also creates a new line struct and chains it
///		to the current line as the p_nextLine, which is actually the "new current last line" (in fact it is the last
///		line by this point) of the file, and initiates its' byte offset poition in the file according to the last 
///		byte offset of the current line.
/// </summary>
/// <param name="file* p_file - A pointer to a file struct represnting the file's lines frames data"></param>
/// <param name="line* p_currentLastLine - A pointer to a file's last known line struct which holds the data concerning the frame in bytes of the line"></param>
/// <returns>A pointer to the new "current last line" if mem alloc is successful, or NULL if it failed.</returns>
static line* prepareAnotherLine(file* p_file, line* p_currentLastLine);
/// <summary>
///  Description - This function takes place after another non-newline single character (=single byte) was read,
///		and simply updates the line and file byte counts(byte_count += 1)
/// </summary>
/// <param name="file* p_file - A pointer to a file struct represnting the file's lines frames data"></param>
/// <param name="line* p_currentLine - A pointer to a file's current examined line"></param>
static void updateLineAndFileFrame(file* p_file, line* p_currentLine);
/// <summary>
///  Description - This function receives a pointer to a file struct and a handle to the input file 
///		which was opened using CreateFile(). The function slowly reads a single byte at a time and updates
///		the file struct accordingly - If it is a newline character, then a new line struct is allocated and
///		added to the nested - list in p_file(all bytes offset fileds are updated as needed). A non - newline character will
///		lead to the simple update of updateLineAndFileFrame(). Otherwise, the file is either empty or the EOF was reached
///		and the function ends. Any failure to allocate dynamic memory terminates the entire program.
/// </summary>
/// <param name="file* p_file - A pointer to a file struct represnting the file's lines frames data"></param>
/// <param name="HANDLE* p_h_inputFileHandle - A pointer to a handle of the input file"></param>
/// <returns></returns>
static BOOL constructFileFrame(file* p_file, HANDLE* p_h_inputFileHandle);







// Functions definitions ---------------------------------------------------------------
file* frameFileLines(char* p_inputFilePath)
{
	file* p_fileData = NULL;
	HANDLE* p_h_inputFileHandle = NULL;
	//Input integrity validation
	if (p_inputFilePath == NULL) {
		printf("Error: Bad inputs to function: %s\n", __func__); return NULL;
	}

	//File struct mem. alloc.
	if (NULL == (p_fileData = fileStructMemoryAllocation()))  return NULL;

	//Opening a Handle to the input file file
	if (NULL == (p_h_inputFileHandle = openFileForInitialRead(p_inputFilePath))) {
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		freeTheFile(p_fileData);
		return NULL; 
	}

	//File(data) struct construction (number of lines, lines lengthes in bytes etc)
	if (STATUS_CODE_SUCCESS != constructFileFrame(p_fileData, p_h_inputFileHandle)) {
		freeTheFile(p_fileData);
		closeHandleProcedure(p_h_inputFileHandle);
		return NULL; 
	}

	//Closing the handle of the initial reading of the file & Freeing the Handle pointer memory (Heap)
	closeHandleProcedure(p_h_inputFileHandle);

	//Returning a pointer of the constructed file struct
	return p_fileData;
}


//......................................Static functions..........................................

static file* fileStructMemoryAllocation()
{
	file* p_fileFrame = NULL;
	//File struct dynamic memory allocation
	if (NULL == (p_fileFrame = (file*)calloc(sizeof(file), SINGLE_OBJECT))) { 
		printf("Error: Failed to allocate memory for a file(frame) struct.\n"); 
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Initial line struct dynamic memory allocation
	if (NULL == (p_fileFrame->p_firstLineInFile = (line*)calloc(sizeof(line), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a line struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		free(p_fileFrame);
		return NULL;
	}

	//Setting the id of the first line & the file (frame) struct number of lines
	p_fileFrame->numberOfLines = 1;

	//Memory allocation for the file (frame) struct & the first line struct has been successful
	return p_fileFrame;
}



static HANDLE* openFileForInitialRead(char* p_filePath)
{
	HANDLE* p_h_inputFileHandle = NULL;
	assert(p_filePath != NULL);

	//Allocating dynamic memory (Heap) for a file Handle pointer
	if (NULL == (p_h_inputFileHandle = (HANDLE*)calloc(sizeof(HANDLE), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a Handle to file '%s'.\n", p_filePath);
		printf("At file: %s\n at line number: %d\n at function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Creating a handle to the input  file
	*p_h_inputFileHandle = CreateFile(
		p_filePath,				// Const null-terminated string describing the file's path
		GENERIC_READ,			// Desired Access is set to Read
		PREVENT_SHARING,		// Share Mode:In the initial run, only the main thread will access the file
		NULL,					// No Security Attributes
		OPEN_EXISTING,			// The file is an existing file. If it doesn't exist we shouldn't open anything
		FILE_ATTRIBUTE_NORMAL,  // General reading in files 
		NULL					// No Template
	);
	//File Handle creation validation
	if (INVALID_HANDLE_VALUE == *p_h_inputFileHandle) {
		printf("Error: Failed to create a Handle to file '%s' for framing, with code: %d.\n", p_filePath, GetLastError());
		free(p_h_inputFileHandle);
		return NULL;
	}
	//Returning file Handle pointer
	return p_h_inputFileHandle;
}





static line* prepareAnotherLine(file* p_fileData, line* p_currentLastLine)
{
	assert(p_fileData != NULL);
	assert(p_currentLastLine != NULL);
	//Memory allocation for the new line
	if (NULL == (p_currentLastLine->p_nextLine = (line*)calloc(sizeof(line), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a line struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}
	//Updating the file(frame) struct with additional line
	p_fileData->numberOfLines += 1;
	//Updating the new line's starting byte position
	(p_currentLastLine->p_nextLine)->initialPositionByByte = p_currentLastLine->initialPositionByByte + p_currentLastLine->numberOfBytesInLine;
	//Returning the new line pointer to become the new current-line
	return p_currentLastLine->p_nextLine;
}

static void updateLineAndFileFrame(file* p_fileData, line* p_currentLine)
{
	assert(p_currentLine != NULL);
	//Updating the total number of bytes in the file - Consider swapping to GetFileSize 
	p_fileData->numberOfBytes += 1;
	//Updating the total number of bytes in the line
	p_currentLine->numberOfBytesInLine += 1;

}

static BOOL constructFileFrame(file* p_fileData, HANDLE* p_h_inputFileHandle)
{
	
	BOOL retVal;
	LPTSTR p_singleByte = NULL;
	DWORD numberOfBytesRead = 0;
	line* p_currentLine = p_fileData->p_firstLineInFile; 
	//Asserts
	assert(p_fileData != NULL);
	assert(p_h_inputFileHandle != NULL);

	//Memory allocation for the single byte buffer
	if (NULL == (p_singleByte = (LPTSTR)calloc(sizeof(TCHAR), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a single-byte buffer.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		printf("Error: Failed to construct the file(frame) struct.\n");
		return STATUS_CODE_FAILURE;
	}



	do {
		//Reading a single byte from the file handle  //CHECK//
		retVal = ReadFile(
			*p_h_inputFileHandle,
			p_singleByte,
			SINGLE_OBJECT,
			&numberOfBytesRead,
			NULL
		);

		

		//Validating a single byte was read correctly
		if (STATUS_FILE_READING_SUCCESS != retVal) { //|| ((retVal == STATUS_FILE_READING_SUCCESS) && (numberOfBytesRead == 0) && (p_file->numberOfBytes = 0))){
			//ReadFile failed if retVal equals 0.
			//FOR NOW IT IS IRRELEVANT - If ReadFile reached EOF at the begining, it returns that the reading status was successful but the number of bytes read will be set to 0.
			printf("Error: Failed to read from the file Handle. Exited with code:%d\n", GetLastError());
			printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
			free(p_singleByte); return STATUS_CODE_FAILURE;
		}
		if ((STATUS_FILE_READING_SUCCESS == retVal) && (0 == numberOfBytesRead)) {
			//If ReadFile reached EOF, it returns that the reading status was successful but the number of bytes read will be set to 0.
			//Validate if the file is emptyAdd case for an empty file..... print and return EMPTY_FILE
			if (p_fileData->numberOfBytes == 0) {
				printf("The input file is empty.\nExiting....\n\n\n\n\n\n");
				return EMPTY_FILE;
			}
			break;
		}
		else {
			//Adding another byte to the current line byte count regardless of what character it is
			updateLineAndFileFrame(p_fileData, p_currentLine);
			if (*p_singleByte == (TCHAR)'\n') {
				//Creating a new line and setting its' ID & initial byte position in the file
				if ((p_currentLine = prepareAnotherLine(p_fileData, p_currentLine)) == NULL) { free(p_singleByte); return STATUS_CODE_FAILURE; }
			}
		}
	} while (numberOfBytesRead != END_OF_FILE); //As long as the number of bytes the operation read, isn't 0(==END_OF_FILE) then we continue reading


	//Freeing the character buffer
	free(p_singleByte);

	//Construction succeeded...
	return STATUS_CODE_SUCCESS;
}