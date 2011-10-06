#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "largeint.h"


large_int* add_s( large_int* first, large_int* second)
{
	int i, new_size, previous = 0;

	large_int* result = (large_int*) malloc(sizeof(large_int));

	/*figure out size of the new integer */
	if( first->size > second->size)
	 	new_size = first->size;
	else
		new_size = second->size;
	
	result->int_array = (unsigned int*) malloc(sizeof(unsigned int) * (new_size + 1));
	

	for(i = 0; i < new_size; i++)
	{
		if(i >= first->size)
			result->int_array[i] = second->int_array[i];
		else if (i >= second->size)
			result->int_array[i] = first->int_array[i];
		else
			result->int_array[i] = first->int_array[i] + second->int_array[i];
		
		if(previous == 1)
			result->int_array[i]++;

		if( i < first->size)
		{	
			if(result->int_array[i] <= first->int_array[i] && previous == 1)
				previous = 1;
			else if (result->int_array[i] < first->int_array[i])
				previous = 1;
			else
				previous = 0;
		}
		else if ( i < second->size)
		{
			if(result->int_array[i] <= second->int_array[i] && previous == 1)
				previous = 1;
			else if (result->int_array[i] < second->int_array[i])
				previous = 1;
			else
				previous = 0;
		}	

	}
	
	if(previous == 1)
	{
		result->int_array[new_size] = 1;
		result->size = new_size+1;
	}
	else
	{
		result->int_array = (unsigned int*) realloc(result->int_array, sizeof(unsigned int) * new_size);
		result->size = new_size;
	}	
	
	return result;
}



