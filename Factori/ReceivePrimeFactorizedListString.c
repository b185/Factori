/* ReceivePrimeFactorizedListString.c
-----------------------------------------------------------------------------------------
	Module Description - This module contains functions meant for solving a given task.
		The main function decompose the task's value into its' prime factors & arranges
		them from smallest to greatest within a primeFactors data struct, while the 
		smallest value is placed at the top of the nested list within the struct, and 
		the greatest value is placed at the bottom of the list. Following that, a routine
		to translate the nested list into a string, containing a task and the prime
		factors it is comprised of, occurs.
		Finally, a task's solution is created in the form of a string.
-----------------------------------------------------------------------------------------
*/

// Library includes ---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <assert.h>


// Projects includes --------------------------------------------------------------------
#include "ReceivePrimeFactorizedListString.h"



// Constants
static const BOOL  STATUS_CODE_FAILURE = FALSE;
static const BOOL  STATUS_CODE_SUCCESS = TRUE;

static const DWORD SINGLE_OBJECT = 1;

static const DWORD NUMBER_OF_TEXT_CHARACTERS_BESIDES_THE_PRIME_FACTORS_CHARACTERS = 20/*words*/ + 5/*spaces*/ + 1/*colon*/ +2/*newline*/;



// Functions decleraitions ---------------------------------------------------------------
/// <summary>
///	 Description - This function allocates memory (on the Heap) for a "primeFactors" struct and returns a pointer to it (calloc)
/// </summary>
/// <returns>A pointer to an initiated 'primeFactors' struct if successful, or NULL if failed.</returns>
static primeFactors* primeFactorsStructMemoryAllocation();
/// <summary>
///	 Description - This function receives an unsigned integer and calculates the number of digits it has. This number is also
///		the number's number of characters it needs for a string representation, and the number of bytes it uses while written in a file
/// </summary>
/// <param name="DWORD number - An unsigned integer"></param>
/// <returns>An unsigned integer representing the number of digits\characters in the number</returns>
static DWORD numberOfCharactersInNumber(DWORD number);
/// <summary>
///	 Description - This function allocates memory (on the Heap) for a "factor" struct and returns a pointer to it (calloc)
/// </summary>
/// <returns>A pointer to an initiated 'factor' struct if successful, or NULL if failed.</returns>
static factor* factorStructMemoryAllocation();
/// <summary>
///	 Description - This function calculates the floor square root of a given number. The input is named "splitted" because generally, this function attempts
///		to receive the root of a number that may have been stripped of his '2's prime factors it is comprised of (splitted). 
///		The algorithm is based on binary search which makes its' running time complexity to log(n).
/// </summary>
/// <param name="DWORD splittedNumber - An unsigned integer number that is meant to be a number that doesn't have '2' as a prime factor (but it may be any number whatsoever)"></param>
/// <returns>An unsigned integer represnting the floor square root of the input number</returns>
static DWORD calculateSquareRootBinary(DWORD splittedNumber);
/// <summary>
///  Description - This function receives a number and calculates the prime factors that the number is comprised of. Then, the function
///		inserts these values into a nested-list, in order from lowest to greatest, and the nested-list's top element ("factor" struct) is pointed
///		to by p_numberFactors's p_firstFactor field. The output is the final status of the operation.
/// </summary>
/// <param name="DWORD number - An unsigned integer number which is a task's value"></param>
/// <param name="primeFactors* p_numberFactors - A pointer to a previously allocated (dynamic memory) 'primeFactors' struct"></param>
/// <returns>A BOOL value representing the function's outcome (The number was dissolved correctly to the prime factors combining it and the values were inserted to p_numberFactors in order for lowest to highest). Success (TRUE) or Failure (False)</returns>
static BOOL calculatePrimeFactors(DWORD number, primeFactors* p_numberFactors);
/// <summary>
///	 Description - This function allocates memory (on the Heap) for a "factorizedString" struct and returns a pointer to it, after
///		setting its' fields with the inputs - a pointer to the string itself, and the string's length.
/// </summary>
/// <param name="DWORD stringLength - An unsigned integer representing the size of the input string"></param>
/// <param name="LPTSTR p_numberPrimeFactorsString - A pointer to a TCHAR string containing formal phrasing of a number and the prime factors it is comprised of, and terminates with '\0'"></param>
/// <returns>A pointer to an initiated 'factorizedString' struct with an updated fields if successful, or NULL if failed.</returns>
static factorizedString* factorizedStringStructMemoryAllocation(DWORD stringLength, LPTSTR p_numberPrimeFactorsString);
/// <summary>
///  Description - This funtion receives as input a number (representing a task's value) and a "primeFactors" struct containing a nested-list of the 
///		values, arranged from lowest in the top of the list to greatest at the bottom of the list, representing the prime factors from which the number is comprised of,
///		and it uses these values to combine a string of the following phrasing "The prime factors of {number} are: {prime-1}, {prime-2}, ..., {prime-n}".
///		This string will later be written to the Tasks file, and represents the solution to the task. 
/// </summary>
/// <param name="primeFactors* p_numberFactors - A pointer to a 'primeFactors' struct, that its' field, p_firstFactor, points the nested-list containing the input number's prime factors"></param>
/// <param name="DWORD number - An unsigned integer number which is a task's value"></param>
/// <returns>A pointer to an initiated 'factorizedString' struct with an updated fields if successful, or NULL if failed.</returns>
static factorizedString* changeFactorsListToFactorsCharacterString(primeFactors* p_numberFactors, DWORD number);
// Functions definitions ---------------------------------------------------------------

factorizedString* receivePrimeFactorizedListString(DWORD number)
{
	primeFactors* p_numberFactors = NULL;
	factorizedString* p_taskPrimeFactorsString = NULL;
	
	//Input integrity validation
	if (0 >= number) {
		printf("Error: Bad inputs to function: %s\n", __func__); return NULL;
	}





	//Initialize a factors struct for the task (number's) factors
	if (NULL == (p_numberFactors = primeFactorsStructMemoryAllocation()))
		return NULL;




	//Calculate the prime factors the task(number) is comprised of & insert them to the prime factors' nested-list
	if (STATUS_CODE_FAILURE == calculatePrimeFactors(number, p_numberFactors)) {
		freeThePrimeFactors(p_numberFactors);
		printf("Error: Failed to calculate the number's factors & arrange them from small to big.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}





	//Construct a string(null terminated characters array) that list all the prime factors of the task
	if (NULL == (p_taskPrimeFactorsString = changeFactorsListToFactorsCharacterString(p_numberFactors, number))) {
		freeThePrimeFactors(p_numberFactors);
		printf("Error: Failed to arrange the number %d as a string of prime factors.\n", number);
		printf("At file: %s\nAt line number: %d\nAt function: %s\n\n\n", __FILE__, __LINE__, __func__);
		return NULL;
	}



	//Free the dynamic allocated for the current task (number)'s primeFactors struct
	freeThePrimeFactors(p_numberFactors);

	//The number's factors were computed & were assembled from smallest to greatest in a factors nested-list struct (from start to end)
	return p_taskPrimeFactorsString;
}



//......................................Static functions..........................................


static primeFactors* primeFactorsStructMemoryAllocation()
{
	primeFactors* p_factors = NULL;

	//Dynamic memory allocation for the primeFactors struct 
	if (NULL == (p_factors = (primeFactors*)calloc(sizeof(primeFactors), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a primeFactors struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Dynamic memory allocation for the first prime factor struct in the nested list within the primeFactors struct
	if (NULL == (p_factors->p_firstFactor = (factor*)calloc(sizeof(factor), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a factor struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		free(p_factors);
		return NULL;
	}

	//Memory allocation succeeded
	return p_factors;
}









static DWORD numberOfCharactersInNumber(DWORD number)
{
	DWORD numberOfCharacters = 0;
	//Assert
	assert(0 <= number); //CHNAGED TO BIGGER\EQUAL (WAS BIGGER)  ---- VALIDATE
	//As long as number is greater than zero we can keep dividing it by 10
	// every division by 10 means the number has another character in its' representation
	while (number != 0) {
		numberOfCharacters++;
		number /= 10;
	}
	return numberOfCharacters;
}
static factor* factorStructMemoryAllocation()
{
	factor* p_factor = NULL;

	//Dynamic memory allocation for the first prime factor struct in the nested list within the primeFactors struct
	if (NULL == (p_factor = (factor*)calloc(sizeof(factor), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a factor struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Memory allocation succeeded
	return p_factor;
}
static DWORD calculateSquareRootBinary(DWORD splittedNumber)
{
	DWORDLONG start = 1, end = 0, middle = 0, floorSqrt = 0;
	//Assert
	assert(0 <= splittedNumber); 

	//Base cases: number equals 1 or 0
	if ((1 == splittedNumber) || (0 == splittedNumber)) return splittedNumber;
	//Explicit typedef to DWORDLONG (Since number may reach as high as 999,999,999 then during binary search,
	// if we pick the half of the number and square it we will go beyond the limit of what an unsigned 32-bit integer can give us
	end = (DWORDLONG)splittedNumber;

	while (start <= end) {
		middle = (start + end) / 2;

		//Case 1: mid is a perfect square to number
		if (middle * middle == (DWORDLONG)splittedNumber) return (DWORD)middle;

		//Case 2: The current mid is smaller than the square root of the number so we move closer to sqrt(number)
		//For the purpose of computing prime factors of a number, the index 'i' in calculateFactors(.) will rise as high
		// as the  floor  of the square root the actual number after stripping it from its' factors '2's
		if (middle * middle < (DWORDLONG)splittedNumber) {
			start = middle + 1;
			floorSqrt = middle;
		}

		//Case 3: mid is greater than square root of the number - mid*mid>sqrt(number)
		else end = middle - 1; //Since we are not interested in the value closest to sqrt(number) from ABOVE it, then we don't update floorSqrt
	}
	return (DWORD)floorSqrt;
}
static BOOL calculatePrimeFactors(DWORD number, primeFactors* p_numberFactors)
{
	DWORD i = 3, squareRootOfNumber = 0; 
	factor* p_prevFactor = NULL, * p_currentFactor = NULL;
	//Asserts
	assert(0 < number);
	assert(NULL != p_numberFactors);
	//Set the first prime factor cell of the primeFactors struct as the current cell
	p_currentFactor = p_numberFactors->p_firstFactor;

	//Special case: number equals 1
	if (1 == number) return STATUS_CODE_SUCCESS;


	//As long as number divisible by 2, insert the value '2' into the number's prime factors nested-list 
	while (number % 2 == 0) {
		//Update number with its' half to continue splitting it to its' prime factors
		number = number / 2;
		//Insert '2' as a factor to the list
		p_currentFactor->factor = 2;
		//Update the primeFactors struct with additional factor count & additional characters to the repersentation string
		p_numberFactors->numberOfPrimeFactors += 1;
		p_numberFactors->numberOfCharacters += 1;
		//Prepare another cell in the list
		p_prevFactor = p_currentFactor;
		if (NULL == (p_currentFactor->p_nextFactor = factorStructMemoryAllocation())) return STATUS_CODE_FAILURE;
		//Advance the number's factors' nested-list
		p_currentFactor = p_currentFactor->p_nextFactor;
	}

	//Calculate the square root of the number after stripping from it its' '2's factors
	squareRootOfNumber = calculateSquareRootBinary(number);
	//Continue to find further factors of the number, which are greater than '2', and are primal numbers themselves
	while (i <= squareRootOfNumber) {
		//Beginning stripping from number its' 'i'ths factors 
		while ((number % i) == 0) {
			//Update number after dividing it with 'i' to remove 'i' from number as a prime factor 
			number = number / i;
			//Insert 'i' as a factor to the list
			p_currentFactor->factor = i;
			//Update the primeFactors struct with additional factor count & additional characters to the repersentation string
			p_numberFactors->numberOfPrimeFactors += 1;
			p_numberFactors->numberOfCharacters += numberOfCharactersInNumber(i);
			//Prepare another cell in the list
			p_prevFactor = p_currentFactor;
			if (NULL == (p_currentFactor->p_nextFactor = factorStructMemoryAllocation())) return STATUS_CODE_FAILURE;
			//Advance the number's factors' nested-list
			p_currentFactor = p_currentFactor->p_nextFactor;
		}
		//Advance 'i' to next odd number higher than 'i' (Because all the even numbers were factors of '2', and they were all removed by removing factors '2's)
		i += 2;
	}

	//Validate if the final number is a prime factor as well
	if (number > 2) {
		p_currentFactor->factor = number;
		//Update the primeFactors struct with additional factor count & additional characters to the repersentation string
		p_numberFactors->numberOfPrimeFactors += 1;
		p_numberFactors->numberOfCharacters += numberOfCharactersInNumber(number);
	}
	else {
		//Freeing the memory of the last cell & setting the cell of the last prime factor of number to NULL
		if (NULL != p_prevFactor) p_prevFactor->p_nextFactor = NULL;
		free(p_currentFactor);
	}

	//The number's factors computation & arranging them in order from smallest to biggest was successful
	return STATUS_CODE_SUCCESS;
}



static factorizedString* factorizedStringStructMemoryAllocation(DWORD stringLength, LPTSTR p_numberPrimeFactorsString)
{
	factorizedString* p_taskFactorizedString = NULL;
	//Asserts


	//Dynamic memory allocation for the first prime factor struct in the nested list within the primeFactors struct
	if (NULL == (p_taskFactorizedString = (factorizedString*)calloc(sizeof(factorizedString), SINGLE_OBJECT))) {
		printf("Error: Failed to allocate memory for a factorizedString struct.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Set the Task's prime factors string's struct fields
	p_taskFactorizedString->p_numberPrimeFactorsString = p_numberPrimeFactorsString;
	p_taskFactorizedString->stringSize = stringLength;

	//Return a pointer to the updated string struct
	return p_taskFactorizedString;
}
static factorizedString* changeFactorsListToFactorsCharacterString(primeFactors* p_numberFactors, DWORD number)
{
	int stringWritingProgress = 0;
	LPTSTR p_numberPrimeFactorsString = NULL;
	DWORD numberOfBytesInThePrimeFactorizedString = 0, i = 0, numberOfCharactersInTask;
	factor* p_currentFactor = NULL;
	factorizedString* p_outputString = NULL;
	//Assert
	assert(NULL != p_numberFactors);

	//Computing the number of character in current Task (number)
	numberOfCharactersInTask = numberOfCharactersInNumber(number);
	//Set the string size according to the number of prime factors comprising the task & number of total characters with which the numbers are represented
	numberOfBytesInThePrimeFactorizedString += NUMBER_OF_TEXT_CHARACTERS_BESIDES_THE_PRIME_FACTORS_CHARACTERS;	//Adding the number of generic characters (Bytes)
	numberOfBytesInThePrimeFactorizedString += numberOfCharactersInTask;										//Adding the task's (number) length in characters (Bytes)
	numberOfBytesInThePrimeFactorizedString += p_numberFactors->numberOfCharacters;								//Adding the total number of characters needed to represent the prime factors
	if (1 != number) numberOfBytesInThePrimeFactorizedString += p_numberFactors->numberOfPrimeFactors * 2 - 1;   //Adding the number of spaces per factor & number of commas 
	//Dynamic memory allocation for the string representing all of the current task's (number) prime factorized string
	if (NULL == (p_numberPrimeFactorsString = (LPTSTR)calloc(sizeof(TCHAR), numberOfBytesInThePrimeFactorizedString + 1))) {
		printf("Error: Failed to allocate memory for the task's prime factorized string.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Initial write to the task's (number) prime factorized string
	if (-1 == (stringWritingProgress = sprintf_s(p_numberPrimeFactorsString, NUMBER_OF_TEXT_CHARACTERS_BESIDES_THE_PRIME_FACTORS_CHARACTERS + numberOfCharactersInTask + 1, "The prime factors of %d are:", number))) {
		free(p_numberPrimeFactorsString);
		printf("Error: Failed to perform the initial writing to the task's prime factorized string.\n");
		printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
		return NULL;
	}

	//Fetch the first Task's smallest prime factor
	p_currentFactor = p_numberFactors->p_firstFactor;
	//Begin main loop to write all of the task's prime factors to the string
	while (i < p_numberFactors->numberOfPrimeFactors) {
		if (stringWritingProgress - 1 == (stringWritingProgress += sprintf_s(p_numberPrimeFactorsString + stringWritingProgress, numberOfCharactersInNumber(p_currentFactor->factor) + 2 + 1, " %d,", p_currentFactor->factor))) {
			free(p_numberPrimeFactorsString);
			printf("Error: Failed to perform a prime factor writing to the task's prime factorized string.\n");
			printf("At file: %s\nAt line number: %d\nAt function: %s", __FILE__, __LINE__, __func__);
			return NULL;
		}
		p_currentFactor = p_currentFactor->p_nextFactor;
		//Advance the factor count
		i += 1;
	}

	//Final write to the task's prime factorized string: Carriage return & Newline
	if (1 == number)  stringWritingProgress++;
	*(p_numberPrimeFactorsString + stringWritingProgress - 1) = '\r';
	*(p_numberPrimeFactorsString + stringWritingProgress) = '\n';
	*(p_numberPrimeFactorsString + stringWritingProgress + 1) = '\0';


	//Construct a factorized string struct (that also holds the string's size)
	if (NULL == (p_outputString = factorizedStringStructMemoryAllocation(numberOfBytesInThePrimeFactorizedString, p_numberPrimeFactorsString))) {
		free(p_numberPrimeFactorsString); return NULL;
	}
	//Task's prime factorized string construction succeeded
	return p_outputString;
}