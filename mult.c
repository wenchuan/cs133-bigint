#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>

#include "largeint.h"

#define ADD add

large_int* mult(large_int* fst, large_int* snd) {
  int i, j, k;
  int n = fst->size;
  int size = fst->size * snd->size;
  uint64_t ll;
  uint32_t *buf;
  uint64_t *accu;

  large_int a, b, *res;
  a.size = fst->size + snd->size; b.size = a.size + 1;
  a.int_array = malloc(a.size * sizeof(unsigned int));
	b.int_array = malloc(b.size * sizeof(unsigned int));

  buf = (uint32_t *)malloc(size * 2 * sizeof(uint32_t));
  accu = (uint64_t *)malloc((fst->size + snd->size) * sizeof(uint64_t));

  assert(buf);
  assert(accu);
  assert(a.int_array);
  assert(b.int_array);

  memset(buf, 0, size * 2 * sizeof(uint32_t));
  memset(accu, 0, (fst->size + snd->size) * sizeof(uint64_t));

  #pragma omp parallel for private(i, j, ll) schedule (dynamic, 32)
  for (i = 0; i < fst->size; i++) {
    for (j = 0; j < snd->size; j++) {
      ll = (uint64_t)fst->int_array[i] * (uint64_t)snd->int_array[j];
      longlong2ints(&ll, buf + j*n*2 + i*2 + 1, buf + j*n*2 + i*2);
    }
  }

  #pragma omp parallel for private(k, i, j) schedule (dynamic, 32)
  for (k = 0; k < fst->size + snd->size; k++) {
    for (i = 0; i < fst->size && i <= k; i++) {
      j = k - i - 1;
      if (j >= 0 && j < snd->size)
        accu[k] += buf[j*n*2 + i*2 + 1];
      j = k - i;
      if (j >= 0 && j < snd->size)
        accu[k] += buf[j*n*2 + i*2];
    }
  }

  b.int_array[0] = 0;
  #pragma omp parallel for private(k, ll) schedule (dynamic, 32)
  for (k = 0; k < fst->size + snd->size; k++) {
    longlong2ints(accu + k, b.int_array + k + 1, a.int_array + k);
  }
  free(buf); free(accu);

  normalize(&a); normalize(&b);
  res = ADD(&a, &b);

	free(a.int_array);
	free(b.int_array);

  normalize(res);

  return res;
}

// Serial version
large_int* mult_s(large_int* fst, large_int* snd) {
  int i, j, k;
  int n = fst->size;
  int size = fst->size * snd->size;
  uint64_t ll;
  uint32_t *buf;
  uint64_t *accu;

  large_int a, b, *res;
  a.size = fst->size + snd->size; b.size = a.size + 1;
  a.int_array = malloc(a.size * sizeof(unsigned int));
	b.int_array = malloc(b.size * sizeof(unsigned int));

  buf = (uint32_t *)malloc(size * 2 * sizeof(uint32_t));
  accu = (uint64_t *)malloc((fst->size + snd->size) * sizeof(uint64_t));

  assert(buf);
  assert(accu);
  assert(a.int_array);
  assert(b.int_array);

  memset(buf, 0, size * 2 * sizeof(uint32_t));
  memset(accu, 0, (fst->size + snd->size) * sizeof(uint64_t));

  for (i = 0; i < fst->size; i++) {
    for (j = 0; j < snd->size; j++) {
      ll = (uint64_t)fst->int_array[i] * (uint64_t)snd->int_array[j];
      longlong2ints(&ll, buf + j*n*2 + i*2 + 1, buf + j*n*2 + i*2);
    }
  }

  for (k = 0; k < fst->size + snd->size; k++) {
    for (i = 0; i < fst->size && i <= k; i++) {
      j = k - i - 1;
      if (j >= 0 && j < snd->size)
        accu[k] += buf[j*n*2 + i*2 + 1];
      j = k - i;
      if (j >= 0 && j < snd->size)
        accu[k] += buf[j*n*2 + i*2];
    }
  }

  b.int_array[0] = 0;
  for (k = 0; k < fst->size + snd->size; k++) {
    longlong2ints(accu + k, b.int_array + k + 1, a.int_array + k);
  }
  free(buf); free(accu);

  normalize(&a); normalize(&b);
  res = add_s(&a, &b);

	free(a.int_array);
	free(b.int_array);

  normalize(res);

  return res;
}
