
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "largeint.h"

#define NUM		9

large_int* Pivots[NUM] = {NULL};
char Sign[NUM] = {0};

void get_pivots(large_int* first, large_int* last)
{
	int i;
	large_int *tmp0, *tmp1, *tmp2, *tmp3;

	for (i = 0; i < NUM; i++)
		if (Pivots[i] != NULL)
			free_li(Pivots[i]);
	
	Pivots[0] = first;
	Pivots[8] = last;
	
	tmp0 = add(Pivots[0], Pivots[8]);
	Pivots[4] = rshift(tmp0, 1);
	free_li(tmp0);

				tmp0 = add(Pivots[4], Pivots[8]);
				Pivots[6] = rshift(tmp0, 1);
				free_li(tmp0);

				tmp1 = add(Pivots[0], Pivots[4]);
				Pivots[2] = rshift(tmp1, 1);
				free_li(tmp1);

				tmp0 = add(Pivots[0], Pivots[2]);
				Pivots[1] = rshift(tmp0, 1);
				free_li(tmp0);

				tmp1 = add(Pivots[2], Pivots[4]);
				Pivots[3] = rshift(tmp1, 1);
				free_li(tmp1);

				tmp2 = add(Pivots[4], Pivots[6]);
				Pivots[5] = rshift(tmp2, 1);
				free_li(tmp2);

				tmp3 = add(Pivots[6], Pivots[8]);
				Pivots[7] = rshift(tmp3, 1);
				free_li(tmp3);
/*
	for (i=0; i<9; i++) {
		printf("get_pivots %d:\n", i);
		print(Pivots[i]);
	}
*/
}


large_int* division(large_int* first, large_int* second)
{
	large_int* result;

	int cmp = compare(first, second);
	if (cmp <= 0) {
		printf("to_do\n");
		return NULL;
	}

	int i, ret;
	int bit_fst = count_bit(first);
	int bit_snd = count_bit(second);
	
	large_int* base = (large_int*) malloc(sizeof(large_int));
	base->size = 1;
	base->int_array = (unsigned int*) malloc(1 * sizeof(unsigned int));
	base->int_array[0] = 1;

	large_int* left = lshift(base, bit_fst - bit_snd - 1);
	large_int* right = lshift(base, bit_fst - bit_snd + 1);

	free_li(base);

	large_int *tmp0, *tmp1;
	// parallel
	get_pivots(left, right);

	int cnt = 0;
	while(1) {
		// parallel
		#pragma omp parallel
		{
		#pragma omp parallel for private(ret, tmp0, tmp1)
		for (i = 1; i < NUM - 1; i++)
		{
			tmp0 = mult(Pivots[i], second);
			ret = compare(tmp0, first);
			if (ret > 0)
				Sign[i] = 'b';
			else {
				tmp1 = add_s(tmp0, second);
				//printf("add %d get:\n", i);
				//print(tmp1);
				ret = compare(tmp1, first);
				if (ret > 0)
					Sign[i] = 'r';
				else
					Sign[i] = 's';
				free_li(tmp1);
			}
			free_li(tmp0);
		}
		}
		// sequential	
		//Sign[0] = 's';
		Sign[NUM-1] = 'b';
		for (i = 1; i < NUM; i++)
		{
			if (Sign[i] == 'r') {
				result = Pivots[i];
				Pivots[i] = NULL;
				goto RET;
			}
			else if (Sign[i] == 'b') 
				break;
		}
	
		tmp0 = Pivots[i-1];
		tmp1 = Pivots[i];
		Pivots[i-1] = NULL;
		Pivots[i] = NULL;

		if (debug) {
			for (i = 0; i < NUM; i++)
				printf("%d:%c ", i, Sign[i]);
			printf("\n");
		}

		get_pivots(tmp0, tmp1);
		cnt++;
	}
	
RET:
	for (i = 0; i < NUM; i++)
		if (Pivots[i] != NULL)
			free_li(Pivots[i]);

		if (debug) printf("count: %d\n", cnt);

	return result;
}


large_int* division_s(large_int* first, large_int* second)
{
	large_int* result;

	int cmp = compare(first, second);
	if (cmp <= 0) {
		printf("to_do\n");
		return NULL;
	}

	int i, ret;
	int bit_fst = count_bit(first);
	int bit_snd = count_bit(second);
	
	large_int* base = (large_int*) malloc(sizeof(large_int));
	base->size = 1;
	base->int_array = (unsigned int*) malloc(1 * sizeof(unsigned int));
	base->int_array[0] = 1;

	large_int* left = lshift(base, bit_fst - bit_snd - 1);
	large_int* right = lshift(base, bit_fst - bit_snd + 1);

	free_li(base);

	large_int *tmp0, *tmp1;
	// parallel
	get_pivots(left, right);

	int cnt = 0;
	while(1) {
		// parallel
		{
		for (i = 1; i < NUM - 1; i++)
		{
			tmp0 = mult(Pivots[i], second);
			ret = compare(tmp0, first);
			if (ret > 0)
				Sign[i] = 'b';
			else {
				tmp1 = add_s(tmp0, second);
				//printf("add %d get:\n", i);
				//print(tmp1);
				ret = compare(tmp1, first);
				if (ret > 0)
					Sign[i] = 'r';
				else
					Sign[i] = 's';
				free_li(tmp1);
			}
			free_li(tmp0);
		}
		}
		// sequential	
		//Sign[0] = 's';
		Sign[NUM-1] = 'b';
		for (i = 1; i < NUM; i++)
		{
			if (Sign[i] == 'r') {
				result = Pivots[i];
				Pivots[i] = NULL;
				goto RET;
			}
			else if (Sign[i] == 'b') 
				break;
		}
	
		tmp0 = Pivots[i-1];
		tmp1 = Pivots[i];
		Pivots[i-1] = NULL;
		Pivots[i] = NULL;

		if (debug) {
			for (i = 0; i < NUM; i++)
				printf("%d:%c ", i, Sign[i]);
			printf("\n");
		}

		get_pivots(tmp0, tmp1);
		cnt++;
	}
	
RET:
	for (i = 0; i < NUM; i++)
		if (Pivots[i] != NULL)
			free_li(Pivots[i]);

		if (debug) printf("count: %d\n", cnt);

	return result;
}

large_int* div_Newton(large_int* A, large_int* B)
{
	int ret;
	int bit_B = count_bit(B);
	int bit_A = count_bit(A);
	int sft = bit_A << 1;

	large_int* base = (large_int*) malloc(sizeof(large_int));
	base->size = 1;
	base->int_array = (unsigned int*) malloc(1 * sizeof(unsigned int));
	base->int_array[0] = 1;

	large_int* est = lshift(base, sft - bit_B);
	large_int* two = lshift(base, sft + 1);

	if (debug) {
		printf("est 0:\n");
		print(est);
	}
	
	large_int *q_est, *tmp_m1, *tmp_s, *tmp_m2, *tmp_old, *tmp;
	int cnt = 0;
	
	while (1)
	{
		tmp_old = est;

		tmp_m1 = mult(est, B);

		tmp_s = sub(two, tmp_m1);

		tmp_m2 = mult(est, tmp_s);

		est = rshift(tmp_m2, sft);
		
		if (debug) {
			printf("est %d:\n", cnt+1);
			print(est);
		}

		free_li(tmp_m1);
		free_li(tmp_s);
		free_li(tmp_m2);

		ret = compare(tmp_old, est);
		free_li(tmp_old);
		
		if (debug)
			printf("compare: %d\n", ret);

		if (ret == 0)
			break;

		cnt++;
	}

		if (debug) printf("count: %d\n", cnt);

	tmp = mult(est, A);
	q_est = rshift(tmp, sft);

	free_li(tmp);

	return q_est;
}


large_int* div_Newton_s(large_int* A, large_int* B)
{
	int ret;
	int bit_B = count_bit(B);
	int bit_A = count_bit(A);
	int sft = bit_A << 1;

	large_int* base = (large_int*) malloc(sizeof(large_int));
	base->size = 1;
	base->int_array = (unsigned int*) malloc(1 * sizeof(unsigned int));
	base->int_array[0] = 1;

	large_int* est = lshift_s(base, sft - bit_B);
	large_int* two = lshift_s(base, sft + 1);

	if (debug) {
		printf("est 0:\n");
		print(est);
	}
	
	large_int *q_est, *tmp_m1, *tmp_s, *tmp_m2, *tmp_old, *tmp;
	int cnt = 0;
	
	while (1)
	{
		tmp_old = est;

		tmp_m1 = mult_s(est, B);

		tmp_s = sub_s(two, tmp_m1);

		tmp_m2 = mult_s(est, tmp_s);

		est = rshift_s(tmp_m2, sft);
		
		if (debug) {
			printf("est %d:\n", cnt+1);
			print(est);
		}

		free_li(tmp_m1);
		free_li(tmp_s);
		free_li(tmp_m2);

		ret = compare(tmp_old, est);
		free_li(tmp_old);
		
		if (debug)
			printf("compare: %d\n", ret);

		if (ret == 0)
			break;

		cnt++;
	}

		if (debug) printf("count: %d\n", cnt);

	tmp = mult_s(est, A);
	q_est = rshift_s(tmp, sft);

	free_li(tmp);

	return q_est;
}


