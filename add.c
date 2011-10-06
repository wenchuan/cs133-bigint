#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>


#include "largeint.h"

int still_carries(int* c, int size)
{
	int i;
	for(i = 0; i < size; i++)
		if(c[i] == 1)
			return 1;
	return 0;
}


large_int* add( large_int* first, large_int* second)
{
	int i, status = 0, cont = 0;
	int *carry1, *carry2;
	large_int* result = (large_int*) malloc(sizeof(large_int));
	
	
	
	if( first->size > second->size)
		result->size = first->size;
	else
		result->size = second->size;
		
	result->int_array = (unsigned int*) malloc(sizeof(unsigned int) * (result->size + 1));
	carry1 = (int*) malloc(sizeof(int)* (result->size + 1));
	carry2 = (int*) malloc(sizeof(int)* (result->size + 1));
	carry1[0] = 0;
	#pragma omp parallel for shared(result, carry1, carry2, first, second) private(i) 
	for(i = 0; i < result->size; i++)
	{
		carry2[i] = 0;
		if( i > first->size)
			result->int_array[i] = second->int_array[i];
		else	if( i > second->size)
			result->int_array[i] = first->int_array[i];
		else
			result->int_array[i] = first->int_array[i] + second->int_array[i];
		if( i < first->size)
		{	

			if (result->int_array[i] < first->int_array[i])
				carry1[i+1] = 1;
			else
				carry1[i+1] = 0;
		}
		else if ( i < second->size)
		{
			if (result->int_array[i] < second->int_array[i])
				carry1[i+1] = 1;
			else
				carry1[i+1] = 0;
		}	
	}
	
	carry2[result->size] = carry1[result->size];
	while(cont == 0)
	{
		cont = 1;
		#pragma omp parallel for shared(result, cont, status, carry1, carry2) private (i) 
		for(i = 0; i < result->size ; i++)
		{
			if(status ==0)
			{	
				result->int_array[i] += carry1[i];
				if( result->int_array[i] == 0 && carry1[i] == 1)
				{	
					carry2[i+1] = 1;
					cont = 0;		
				}	
				carry1[i] = 0;
			}
			else
			{
				result->int_array[i] += carry2[i];
				if( result->int_array[i] == 0 && carry2[i] == 1)
				{
					carry1[i+1] = 1;
					cont = 0;
				}
				carry2[i] = 0;
			}	
		}
		if(status == 0)
			status = 1;
		else
			status = 0;

	}
	if(status == 0)
	{
		
		if(carry1[result->size] == 1)
		{
			i = 0;
			i = 0;
			result->int_array[result->size] = 1;
			result->size++;
		}
		else
		{
			i = 1;
			i = 1;
			result->int_array = (unsigned int*) realloc(result->int_array, sizeof(unsigned int)* result->size);
		} 
	
	}
	if(status == 1)
	{
		if(carry2[result->size] == 1)
		{
			i=0;
			i=0;
			result->int_array[result->size] = 1;
			result->size++;
		}
		else
		{
			i=1;
			i=1;
			result->int_array = (unsigned int*) realloc(result->int_array, sizeof(unsigned int)* result->size);
		}
	}






	free(carry1);
	free(carry2);	
	
/*
	large_int *gmp = add_gmp(first,second);
	if(compare(gmp,result) != 0)
	{
		printf("a = %s\n b = %s\n c = %s\n d = %s\n",
			print_hex(first),
			print_hex(second),
			print_hex(result),
			print_hex(gmp));
		assert(0 == 1);
	}
*/

	return result;
	
}
