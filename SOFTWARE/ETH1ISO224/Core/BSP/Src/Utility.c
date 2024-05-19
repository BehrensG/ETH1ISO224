/*
 * utility.c
 *
 *  Created on: Mar 22, 2024
 *      Author: grzegorz
 */

#include <string.h>
//#include <stdio.h>
#include <ctype.h>

#include "Utility.h"
#include "ADC.h"
#include "FloatToString.h"

extern float measurements[];

inline bool UTIL_Timeout(uint32_t start, uint32_t timeout)
{
	if (((HAL_GetTick() - start) >= timeout) || (0U == timeout))
	{
		return false;
	}
	else
	{
		return true;
	}
}




char buffer[ADC_MEASUREMENT_BUFFER * UTIL_ASCII_SIZE +1];

char* UTIL_FloatArrayToASCII(float* float_array, uint32_t num_floats)
{

    size_t size;
    size_t sum = 0;

    for (uint32_t x = 0; x < num_floats; x++)
    {
    	size = floatToString(buffer + sum, float_array[x]);
    	sum +=size;

    }


    buffer[sum - 1] = '\0'; // Delete last ','

    return buffer;
}

int32_t UTIL_WhiteSpace(const char* string, uint32_t size)
{
	for (int32_t i = 0; i < size; i++)
	{
	        if (isspace((unsigned char)string[i]))
	        {
	            return i;
	        }
	    }

	return -1;
}

char* UTIL_FloatArrayToREAL(float* float_array, uint32_t num_floats)
{

    uint32_t size = num_floats * sizeof(float); // +1 for the null terminator



    memcpy(buffer, float_array, size);

    buffer[size] = '\0';

    return buffer;
}
