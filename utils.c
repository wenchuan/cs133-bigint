#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <omp.h>
#include <gmp.h>

#include "largeint.h"

void longlong2ints(uint64_t *ll, uint32_t *h, uint32_t *l) {
  *l = (*ll) & UINT_MAX;
  *h = (*ll >> 32) & UINT_MAX;
}

int count_bit(large_int* li) {
	unsigned tmp = li->int_array[li->size-1];
	int shift = 0;
	while (tmp) {
		tmp = tmp >> 1;
		shift++;
	}
	return (li->size-1) * 32 + shift;
}

void free_li(large_int* li) {
	if (li) {
		free(li->int_array);
		free(li);
	}
	li = NULL;
}

void normalize(large_int *li) {
  assert(li);
  int i = li->size - 1;
  for (; i > 0; i--)
    if (li->int_array[i] != 0)
      break;
  li->size = i + 1;
  li->int_array = realloc(li->int_array, li->size * sizeof(uint32_t));
}

// Shift our large number by N 32-bit to the right,
// so if number is abcd it becomes ab when shifted by 2
void shift_right(large_int* a, int n) {
  unsigned int* newint;
  int newsize=a->size-n;
  newint=malloc(sizeof(unsigned int)*newsize);
  if (newint==NULL) die("memory allocation error");
  memcpy(newint,a->int_array+sizeof(unsigned int)*(size_t)n, newsize*sizeof(unsigned int));
  free(a->int_array);
  a->int_array=newint;
  a->size=newsize;
}

// Shifts our large number by N 32-bit to the left
// so if number is abcd it becomes abcd00
void shift_left(large_int* a, int n) {
  int newsize=a->size+n;
  unsigned int* newarray=malloc(sizeof(unsigned int)*newsize);
  if (newarray==NULL) die("memory allocation error");
  bzero(newarray,sizeof(unsigned int)*n);
  memcpy((char*)newarray+sizeof(unsigned int)*(size_t)n,a->int_array,sizeof(unsigned int)*a->size);
  free(a->int_array);
  a->int_array=newarray;
  a->size=newsize;
}

void shift_left2(large_int* a, int n) {
  large_int* num;
  num=lshift(a,n*32);
  /* if (debug) { printf("lshift called to shift a (size=%d) by %d bits: ",a->size,n*32);
	       print_hex(a);
	       printf("result is num (size=%d): ",num->size);
	       print_hex(num);
	     }
  */
  free(a->int_array);
  a->size=num->size;
  a->int_array=num->int_array;
  free(num);
}

// Utility function that creates a copy of a large number - allocates memory and copies
large_int* large_int_copy(large_int* a) {
  large_int* res;
  res=malloc(sizeof(large_int));
  if (res==NULL) die("memory error");
  res->size=a->size;
  res->sign=a->sign;
  res->int_array=malloc(sizeof(unsigned int)*res->size);
  if (res->int_array==NULL) die("memory error");
  memcpy(res->int_array, a->int_array, sizeof(unsigned int)*res->size);
  return res;
}

// Utility function that generates random large number of given size
large_int* random_num(int size) {
  large_int* res;
  int i;
  res=malloc(sizeof(large_int));
  res->size=size;
  res->sign=1;
  res->int_array=malloc(sizeof(unsigned int)*size);
  if (res->int_array==NULL) die("memory error");
  for(i=0;i<size;i++) res->int_array[i]=rand();
  return res;
}

int test_s(large_int* first, large_int* second) {
	if (first->size > second->size)
		return 0;
	if (first->size < second->size)
		return 0;
	int i;
	for (i = first->size-1; i >= 0; i--)
	{
		if (first->int_array[i] == second->int_array[i])
			continue;
		else if (first->int_array[i] > second->int_array[i])
			return 0;
		else if (first->int_array[i] < second->int_array[i])
			return 0;
	}
	return 1;
}

int test(large_int* first, large_int* second) {
	if (first->size > second->size)
		return 0;
	if (first->size < second->size)
		return 0;

	char* sign = (char*) malloc(first->size);
	memset(sign, 0, first->size);

	int i;
	#pragma omp parallel for
	for (i = 0; i < first->size; i++)
	{
		if (first->int_array[i] != second->int_array[i])
			sign[i] = 1;
	}

	for (i = 0; i < first->size; i++)
	{
		if (sign[i])
			return 0;
	}
	return 1;
}


int compare(large_int* first, large_int* second) {
        if (first->size > second->size)
                return 1;
        if (first->size < second->size)
                return -1;
        int i;
        for (i = first->size-1; i >= 0; i--)
        {
                if (first->int_array[i] == second->int_array[i])
                        continue;
                else if (first->int_array[i] > second->int_array[i])
                        return 1;
                else if (first->int_array[i] < second->int_array[i])
                        return -1;
        }
        return 0;
}

void print(large_int *li) {
  int i;
  for (i = 0; i < li->size; i++)
    printf("%d: %u\n", i, li->int_array[i]);
}

void print_hex(large_int *li) {
  int i;
  for (i = li->size - 1; i >= 0; i--)
    printf("%08x", li->int_array[i]);
  printf("\n");
}

void print_dec(large_int *li) {
  int i;
  char buf[li->size * 8 + 1];
  mpz_t integ;
  mpz_init(integ);
  for (i = 0; i < li->size; i++)
    sprintf(buf + i*8, "%08x", li->int_array[li->size - i - 1]);
  gmp_sscanf(buf, "%Zx", integ);
  gmp_printf("%Zd\n", integ);
  mpz_clear(integ);
}

large_int *get_random(int size) {
  // return pointer and the pointer inside must be freed by caller
  assert(size > 0);
  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = size;
  res->int_array = (unsigned int *)malloc(size * sizeof(int));
  assert(res->int_array);

  int i;
  uint16_t *p = (uint16_t *)res->int_array;
  for (i = 0; i < size * 2; i++)
    p[i] = rand() % 65536;
  return res;
}

large_int *get_hex(const char *filename) {
  assert(filename);
  if (strcmp(filename+strlen(filename)-4, ".hex") != 0) {
    fprintf(stderr, "get_hex: only take in .hex text file with lower case hexadecimal number.\n");
    return NULL;
  }
  mpz_t num; mpz_init(num);
  int fsize, size, int_size;
  int i, j, idx;
  char *buf, ch;

  FILE *fp = fopen(filename, "r");
  gmp_fscanf(fp, "%Zx", num);
  fsize = ftell(fp);
  fclose(fp);

  buf = (char *)malloc(fsize + 8);
  assert(buf);

  gmp_sprintf(buf, "%Zx", num);
  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(num); free(buf);

  return res;
}

large_int *get_dec(const char *filename) {
  assert(filename);
  if (strcmp(filename+strlen(filename)-4, ".dec") != 0) {
    fprintf(stderr, "get_dec: only take in .dec text file with lower case decimal number.\n");
    return NULL;
  }
  mpz_t num; mpz_init(num);
  int fsize, size, int_size;
  int i, j, idx;
  char *buf, ch;

  FILE *fp = fopen(filename, "r");
  gmp_fscanf(fp, "%Zd", num);
  fsize = ftell(fp);
  fclose(fp);

  buf = (char *)malloc(fsize + 8);
  assert(buf);

  gmp_sprintf(buf, "%Zx", num);
  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(num); free(buf);

  return res;
}

large_int *regenerate(large_int *li) {
  // no allocation
  assert(li);
  assert(li->int_array);

  int i;
  uint16_t *p = (uint16_t *)li->int_array;
  for (i = 0; i < li->size * 2; i++)
    p[i] = rand() % 65536;
  return li;
}

large_int* add_gmp(large_int *fst, large_int* snd) {
  mpz_t f, s;
  int i, j;
  char *buf;
  int size, int_size, idx;
  uint32_t ch;
  size = fst->size > snd->size ? fst->size : snd->size;
  size = size * 8 + 9;
  buf = (char *)malloc(size);

  mpz_init(f); mpz_init(s);

  memset(buf, 0, size);
  for (i = 0; i < fst->size; i++)
    sprintf(buf + i*8, "%08x", fst->int_array[fst->size - i - 1]);
  if (debug)
    printf("add_gmp: buf of a: %s\n", buf);
  gmp_sscanf(buf, "%Zx", f);
  memset(buf, 0, size);
  for (i = 0; i < snd->size; i++)
    sprintf(buf + i*8, "%08x", snd->int_array[snd->size - i - 1]);
  gmp_sscanf(buf, "%Zx", s);
  if (debug)
    printf("add_gmp: buf of b: %s\n", buf);
  memset(buf, 0, size);

  mpz_add(s, f, s);

  gmp_sprintf(buf, "%Zx", s);
  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(f); mpz_clear(s); free(buf);

  return res;
}

large_int* sub_gmp(large_int *fst, large_int* snd) {
  if (debug) {
    printf("sub_gmp: fst: "); print_hex(fst);
    printf("sub_gmp: snd: "); print_hex(snd);
  }
  mpz_t f, s;
  int i, j;
  char *buf;
  int size, int_size, idx;
  uint32_t ch;
  size = fst->size > snd->size ? fst->size : snd->size;
  size = size * 8 + 9;
  buf = (char *)malloc(size);

  mpz_init(f); mpz_init(s);

  memset(buf, 0, size);
  for (i = 0; i < fst->size; i++)
    sprintf(buf + i*8, "%08x", fst->int_array[fst->size - i - 1]);
  if (debug)
    printf("sub_gmp: buf of a: %s\n", buf);
  gmp_sscanf(buf, "%Zx", f);
  memset(buf, 0, size);
  for (i = 0; i < snd->size; i++)
    sprintf(buf + i*8, "%08x", snd->int_array[snd->size - i - 1]);
  gmp_sscanf(buf, "%Zx", s);
  if (debug)
    printf("sub_gmp: buf of b: %s\n", buf);
  memset(buf, 0, size);

  if (debug) {
    gmp_printf("sub_gmp: a: %Zd\n", f);
    gmp_printf("sub_gmp: a: %Zx\n", f);
    gmp_printf("sub_gmp: b: %Zd\n", s);
    gmp_printf("sub_gmp: b: %Zx\n", s);
  }

  mpz_sub(s, f, s);

  gmp_sprintf(buf, "%Zx", s);
  if (debug) {
    gmp_printf("sub_gmp: %Zd\n", s);
    gmp_printf("sub_gmp: %Zx\n", s);
  }

  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(f); mpz_clear(s); free(buf);

  return res;
}

large_int* mult_gmp(large_int *fst, large_int* snd) {
  mpz_t f, s;
  int i, j;
  char *buf;
  int size, int_size, idx;
  uint32_t ch;
  size = fst->size * snd->size;
  size = size * 8 + 9;
  buf = (char *)malloc(size);

  mpz_init(f); mpz_init(s);

  memset(buf, 0, size);
  for (i = 0; i < fst->size; i++)
    sprintf(buf + i*8, "%08x", fst->int_array[fst->size - i - 1]);
  gmp_sscanf(buf, "%Zx", f);
  memset(buf, 0, size);
  for (i = 0; i < snd->size; i++)
    sprintf(buf + i*8, "%08x", snd->int_array[snd->size - i - 1]);
  gmp_sscanf(buf, "%Zx", s);
  memset(buf, 0, size);

  mpz_mul(s, f, s);

  gmp_sprintf(buf, "%Zx", s);
  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(f); mpz_clear(s); free(buf);

  return res;
}

// fst / snd
large_int* division_gmp(large_int *fst, large_int* snd) {
  mpz_t f, s;
  int i, j;
  char *buf;
  int size, int_size, idx;
  uint32_t ch;
  size = fst->size > snd->size ? fst->size : snd->size;
  size = size * 8 + 9;
  buf = (char *)malloc(size);

  mpz_init(f); mpz_init(s);

  memset(buf, 0, size);
  for (i = 0; i < fst->size; i++)
    sprintf(buf + i*8, "%08x", fst->int_array[fst->size - i - 1]);
  gmp_sscanf(buf, "%Zx", f);
  memset(buf, 0, size);
  for (i = 0; i < snd->size; i++)
    sprintf(buf + i*8, "%08x", snd->int_array[snd->size - i - 1]);
  gmp_sscanf(buf, "%Zx", s);
  memset(buf, 0, size);

  mpz_fdiv_q(f, f, s);

  gmp_sprintf(buf, "%Zx", f);
  size = strlen(buf);
  int_size = (size + 7) / 8;

  large_int *res = (large_int *)malloc(sizeof(large_int));
  assert(res);
  res->size = int_size;
  res->int_array = (unsigned int *)malloc(int_size * sizeof(uint32_t));
  memset(res->int_array, 0, int_size * sizeof(uint32_t));
  assert(res->int_array);

  for (i = 0; i < int_size; i++) {
    for (j = 7; j >= 0; j--) {
      idx = size - 1 - (i * 8 + j);
      if (idx >= 0) {
        ch = buf[idx] <= '9' ? buf[idx] - '0' : buf[idx] - 'a' + 10;
        res->int_array[i] = res->int_array[i] * 16 + ch;
      }
    }
  }

  mpz_clear(f); mpz_clear(s); free(buf);

  return res;
}
