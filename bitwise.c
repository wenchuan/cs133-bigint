
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "largeint.h"

large_int* rsft (large_int* first, large_int* second)
{
	int snd = second->int_array[0];
	return rshift(first, snd);
}

large_int* rsft_s (large_int* first, large_int* second)
{
	int snd = second->int_array[0];
	return rshift_s(first, snd);
}

large_int* lsft (large_int* first, large_int* second)
{
	int snd = second->int_array[0];
	return lshift(first, snd);
}

large_int* lsft_s (large_int* first, large_int* second)
{
	int snd = second->int_array[0];
	return lshift_s(first, snd);
}

large_int* rshift (large_int* first, int second)
{
	normalize(first);

	int i;
	int size = first->size;
	int sft_int, sft_bit, lsft_bit;
		
	sft_int = second / 32;
	sft_bit = second % 32;
	lsft_bit = 32 - sft_bit;

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	memset(result->int_array + size - (sft_int + 1), 0, (sft_int + 1) * sizeof(unsigned int));

	#pragma omp parallel for
	for (i = sft_int; i < size - 1; i++)
	{
		result->int_array[i-sft_int] = first->int_array[i] >> sft_bit;
		if (sft_bit != 0)
			result->int_array[i-sft_int] |= first->int_array[i+1] << lsft_bit;
	}

	result->int_array[size-1-sft_int] = first->int_array[size-1] >> sft_bit;
	normalize(result);

	return result;
}

large_int* rshift_s (large_int* first, int second)
{
	normalize(first);

	int i;
	int size = first->size;
	int sft_int, sft_bit, lsft_bit;
		
	sft_int = second / 32;
	sft_bit = second % 32;
	lsft_bit = 32 - sft_bit;

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	memset(result->int_array + size - (sft_int + 1), 0, (sft_int + 1) * sizeof(unsigned int));

	for (i = sft_int; i < size - 1; i++)
	{
		result->int_array[i-sft_int] = first->int_array[i] >> sft_bit;
		if (sft_bit != 0)
			result->int_array[i-sft_int] |= first->int_array[i+1] << lsft_bit;
	}

	result->int_array[size-1-sft_int] = first->int_array[size-1] >> sft_bit;
	normalize(result);

	return result;
}

large_int* lshift (large_int* first, int second)
{
	normalize(first);

	int i;
	int size = first->size;
	int sft_int, sft_bit, rsft_bit;
		
	sft_int = second / 32;
	sft_bit = second % 32;
	rsft_bit = 32 - sft_bit;

	int new_size = size + sft_int + 1;

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = new_size;
	result->int_array = (unsigned int*) malloc(new_size* sizeof(unsigned int));
	memset(result->int_array, 0, (sft_int + 1) * sizeof(unsigned int));
	memset(result->int_array + new_size -(sft_int + 1), 0, (sft_int + 1) * sizeof(unsigned int));

	#pragma omp parallel for
	for (i = 1; i < size; i++)
	{
		result->int_array[i+sft_int] = first->int_array[i] << sft_bit;
		if (debug) printf("lshift - first: %u result: %u\n", first->int_array[i], result->int_array[i+sft_int]);

		if (sft_bit != 0)
			result->int_array[i+sft_int] |= first->int_array[i-1] >> rsft_bit;
	}
	
	result->int_array[sft_int] = first->int_array[0] << sft_bit;
	normalize(result);
	
	return result;
}

large_int* lshift_s (large_int* first, int second)
{
	normalize(first);

	int i;
	int size = first->size;
	int sft_int, sft_bit, rsft_bit;
		
	sft_int = second / 32;
	sft_bit = second % 32;
	rsft_bit = 32 - sft_bit;

	int new_size = size + sft_int + 1;

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = new_size;
	result->int_array = (unsigned int*) malloc(new_size* sizeof(unsigned int));
	memset(result->int_array, 0, (sft_int + 1) * sizeof(unsigned int));
	memset(result->int_array + new_size -(sft_int + 1), 0, (sft_int + 1) * sizeof(unsigned int));

	for (i = 1; i < size; i++)
	{
		result->int_array[i+sft_int] = first->int_array[i] << sft_bit;
		if (debug) printf("lshift - first: %u result: %u\n", first->int_array[i], result->int_array[i+sft_int]);

		if (sft_bit != 0)
			result->int_array[i+sft_int] |= first->int_array[i-1] >> rsft_bit;
	}
	
	result->int_array[sft_int] = first->int_array[0] << sft_bit;
	normalize(result);
	
	return result;
}


large_int* bit_and (large_int* first, large_int* second)
{
	int i;
	int size, size_s;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
	}		
	else {
		size = second->size;
		size_s = first->size;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	
	#pragma omp parallel for
	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] & second->int_array[i];
		else
			result->int_array[i] = 0;
	}		
	
	return result;
}

large_int* bit_and_s (large_int* first, large_int* second)
{
	int i;
	int size, size_s;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
	}		
	else {
		size = second->size;
		size_s = first->size;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	
	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] & second->int_array[i];
		else
			result->int_array[i] = 0;
	}		
	
	return result;
}


large_int* bit_or (large_int* first, large_int* second)
{
	int i;
	int size, size_s, is_first;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
		is_first = 1;
	}		
	else {
		size = second->size;
		size_s = first->size;
		is_first = 0;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	
	#pragma omp parallel for
	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] | second->int_array[i];
		else
			result->int_array[i] =  is_first ? first->int_array[i] : second->int_array[i];
	}		
	
	return result;
}


large_int* bit_or_s (large_int* first, large_int* second)
{
	int i;
	int size, size_s, is_first;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
		is_first = 1;
	}		
	else {
		size = second->size;
		size_s = first->size;
		is_first = 0;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));
	
	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] | second->int_array[i];
		else
			result->int_array[i] =  is_first ? first->int_array[i] : second->int_array[i];
	}		
	
	return result;
}


large_int* bit_xor (large_int* first, large_int* second)
{
	int i;
	int size, size_s, is_first;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
		is_first = 1;
	}		
	else {
		size = second->size;
		size_s = first->size;
		is_first = 0;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));

	#pragma omp parallel for
	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] ^ second->int_array[i];
		else
			result->int_array[i] =  is_first ? first->int_array[i] : second->int_array[i];
	}		
	
	return result;
}

large_int* bit_xor_s (large_int* first, large_int* second)
{
	int i;
	int size, size_s, is_first;

	if (first->size > second->size) {
		size = first->size;
		size_s = second->size;
		is_first = 1;
	}		
	else {
		size = second->size;
		size_s = first->size;
		is_first = 0;
	}

	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));

	for (i = 0; i < size; i++)
	{
		if (i < size_s)
			result->int_array[i] = first->int_array[i] ^ second->int_array[i];
		else
			result->int_array[i] =  is_first ? first->int_array[i] : second->int_array[i];
	}		
	
	return result;
}


large_int* bit_not (large_int* first)
{
	int i, size = first->size;
	
	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));

	#pragma omp parallel for
	for (i = 0; i < size; i++)
	{
		result->int_array[i] = ~ first->int_array[i];
	}
	
	return result;
}

large_int* bit_not_s (large_int* first)
{
	int i, size = first->size;
	
	large_int* result = (large_int*) malloc(sizeof(large_int));
	result->size = size;
	result->int_array = (unsigned int*) malloc(size * sizeof(unsigned int));

	for (i = 0; i < size; i++)
	{
		result->int_array[i] = ~ first->int_array[i];
	}
	
	return result;
}



