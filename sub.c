#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "largeint.h"

/*assumption: 	subtraction would not be called in a situation where
			a negative number would be produced*/



large_int* sub( large_int* first, large_int* second)
{
	large_int* result = (large_int *) malloc(sizeof(large_int));
	unsigned long long temp;
	int *borrow1, *borrow2;
	int i, j = 0, status = 0, cont = 0 ;
	omp_lock_t borrow_lock;	
	
	
	
	omp_init_lock(&borrow_lock);
	result->size = first->size;
	borrow1 = (int*) malloc(sizeof(int) * (result->size+1));
	borrow2 = (int*) malloc(sizeof(int) * (result->size+1));
	
	
	result->int_array = (unsigned int*) malloc(sizeof(unsigned int) * result->size);

        borrow1[0] = 0;

	do
	{

		if(j == 0)
		{
			#pragma omp parallel for shared(result, borrow1) private(temp, i) schedule(dynamic, 35)
			for(i = 0; i < result->size - 1; i++)
			
			{
				borrow2[i] = 0;
				if (i >= second->size)
				{
					borrow1[i+1] = 0;
					result->int_array[i] = first->int_array[i];
				}
				else
				{
					if(first->int_array[i] < second->int_array[i])
					{
						borrow1[i+1] = 1;
						temp = 4294967296ULL + (unsigned long long) first->int_array[i];
						result->int_array[i] = temp - (unsigned long long) second->int_array[i];				
					}
					else
					{
						borrow1[i+1] = 0;
						result->int_array[i] = first->int_array[i] - second->int_array[i];
					}
				}
			}
			if (result->size > second->size)
			{
				result->int_array[result->size - 1 ] = first->int_array[result->size - 1];
			}
			else
			{
				result->int_array[result->size -1] = first->int_array[result->size -1] - second->int_array[result->size -1];
			}
			cont =1;
		}

		else
		{
			cont = 0;
		
			if(status == 0)
			{
				#pragma omp parallel for shared (result, borrow2, borrow1, cont, status) private (i)
				for( i = 0; i < result->size; i++)
				{	
	
					if(result->int_array[i] == 0 && borrow1[i] == 1)
					{
						borrow2[i+1] = 1;
						cont = 1;
					}
					result->int_array[i] -= borrow1[i];	
					borrow1[i] = 0;
				}
				status = 1;
			}
			else
			{	
				#pragma omp parallel for shared (result, borrow2, borrow1, cont, status) private (i)
				for( i = 0; i < result->size; i++)
				{
					if(result->int_array[i] == 0 && borrow2[i] == 1)
					{
						borrow1[i+1] = 1;
						cont = 1;
					}
					result->int_array[i] -= borrow2[i];
					borrow2[i] = 0;
				}
				status = 0;
			}
		}
		j++;
	
	}while (cont == 1);
	
	i = 5;
	
	free(borrow1);
	free(borrow2);
	normalize(result);
	return result;
	
}

