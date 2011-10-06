#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "largeint.h"

large_int* sub_s( large_int* f, large_int* second)
{
	int i, j, new_size;
	unsigned long long temp;
	large_int* first = large_int_copy(f);



	new_size = first->size;
	large_int* result = (large_int*) malloc(sizeof(large_int));
	
	
	
	result->int_array = (unsigned int*) malloc(sizeof(unsigned int) * new_size);
		

	for( i = 0; i < new_size; i++)
	{
		if(i >= second->size)
			result->int_array[i] = first->int_array[i];
		else
			result->int_array[i] = first->int_array[i] - second->int_array[i];
		
		if(result->int_array[i] > first->int_array[i])
		{
			temp = 4294967296ULL + (unsigned long long) first->int_array[i];
			result->int_array[i] = (unsigned int)(temp - (unsigned long long)second->int_array[i]);
			if( first->int_array[i+1] == 0)
			{
				for( j = i+1; first->int_array[j] == 0; j++)
				{
					first->int_array[j]--;
				}
				
				first->int_array[j]--;
			}
			else
			{
				
				first->int_array[i+1]--;
			}
				
		}

	}
	
	
	result->size = new_size;
	normalize(result);
	return result;
}	



