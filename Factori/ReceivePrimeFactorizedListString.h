/* ReceivePrimeFactorizedListString.h
------------------------------------------------------------------------------
	Module Description - header module for ReceivePrimeFactorizedListString.c
------------------------------------------------------------------------------
*/


#pragma once
#ifndef __RECEIVE_PRIME_FACTORIZED_LIST_STRING_H__
#define __RECEIVE_PRIME_FACTORIZED_LIST_STRING_H__


// Library includes -------------------------------------------------------
#include <Windows.h>


// Projects includes ------------------------------------------------------
#include "HardCodedData.h"
#include "MemoryHandling.h"


//Functions Declarations

/// <summary>
/// Description - This function receives an unsigned integer representing a task's value, solves it by computing the
///		prime factors it is comprised of, and arranges the number and its' prime factors in a string with the following 
///		formality : "The prime factors of {number} are: {prime-1}, {prime-2}, ..., {prime-n}", from smallest to greatest. 
///		A pointer to a "factorizedString" struct, that one of its' fields points at the string, and the other contains the
///		string's size, is returned as output.
/// </summary>
/// <param name="DWORD number - An unsgined integer representing a task's value"></param>
/// <returns>A pointer to a 'factorizedString' containing the solution of the inputted task if successful, or NULL if anything failed (mainly mem. alloc.)</returns>
factorizedString* receivePrimeFactorizedListString(DWORD number);


#endif //__RECEIVE_PRIME_FACTORIZED_LIST_STRING_H__
