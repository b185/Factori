/* FrameFileLines.h
------------------------------------------------------------
	Module Description - Header module for FrameFileLines.c
------------------------------------------------------------
*/


#pragma once
#ifndef __FRAME_FILE_LINES_H__
#define __FRAME_FILE_LINES_H__


// Library includes ---------------------------------------------
#include <Windows.h>



// Projects includes --------------------------------------------
#include "HardCodedData.h"
#include "MemoryHandling.h"


//Functions Declarations
/// <summary> 
///  Description - This function receives the input file path as input, and uses it 
///		to open the input file(with a Handle) in order to map the file into lines. These lines are described by
///		the initial byte offset(byte position) in the file, and number of bytes in the line until a newline character
///		appears('\n'). The lines data is stored in "line"s structs, and all of them are connected as a nested-list
///		within a "file" struct that contains additional data e.g.total number of lines in the file.
///		Evantually, a "file" struct updated with	all of the file data is returned to main.
/// </summary>
/// <param name="char* p_inputFilePath - A pointer to the commandline argument (string) that represents the input file file's path (absolute\relative)"></param>
/// <returns>A pointer to an updated, dynamicaly allocated "file" struct that contains all the byte-offsets-data of the input file if successful, or NULL if failed.</returns>
file* frameFileLines(char* p_inputFilePath);


#endif //__FRAME_FILE_LINES_H__
