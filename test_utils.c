#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#include "largeint.h"

void die(char* message) {
  printf("Error: %s\n",message);
  exit(1);
}

oper str2oper(char* str) {
  if (!strcmp("add",str)) return P_ADD;
  if (!strcmp("add_s",str)) return S_ADD;
  if (!strcmp("sub",str)) return P_SUB;
  if (!strcmp("sub_s",str)) return S_SUB;
  if (!strcmp("mul",str)) return P_MUL;
  if (!strcmp("mul_s",str)) return S_MUL;
  if (!strcmp("kmul",str)) return P_KMUL;
  if (!strcmp("kmul_s",str)) return S_KMUL;
  if (!strcmp("div",str)) return P_DIV;
  if (!strcmp("div_s",str)) return S_DIV;
  if (!strcmp("ndiv",str)) return P_NDIV;
  if (!strcmp("ndiv_s",str)) return S_NDIV;
  if (!strcmp("lsft",str)) return P_LSFT;
  if (!strcmp("lsft_s",str)) return S_LSFT;
  if (!strcmp("rsft",str)) return P_RSFT;
  if (!strcmp("rsft_s",str)) return S_RSFT;
  if (!strcmp("and",str)) return P_AND;
  if (!strcmp("and_s",str)) return S_AND;
  if (!strcmp("or",str)) return P_OR;
  if (!strcmp("or_s",str)) return S_OR;
  if (!strcmp("xor",str)) return P_XOR;
  if (!strcmp("xor_s",str)) return S_XOR;
  if (!strcmp("not",str)) return P_NOT;
  if (!strcmp("not_s",str)) return S_NOT;
  if (!strcmp("tst",str)) return P_TST;
  if (!strcmp("tst_s",str)) return S_TST;
  return P_UNKNOWN;
}

void find_timediff(struct timespec ts_start, struct timespec ts_end, long long *tnsec, double* sec, long* tsec, long* tmsec) {
  long long elapsed_time;
  long long tdiff = 1000;
  tdiff*=1000;
  tdiff*=1000;
  elapsed_time=(ts_end.tv_sec-ts_start.tv_sec)*tdiff;
  long long ex = ts_end.tv_nsec-ts_start.tv_nsec;
  elapsed_time += ex;
  *tsec=elapsed_time/tdiff;
  *sec=(double)elapsed_time/(double)tdiff;
  *tmsec=(elapsed_time-*tsec*tdiff)/1000;
  *tnsec=elapsed_time;
}

large_int* int_large_int(int i) {
  large_int* result;
  result=malloc(sizeof(large_int));
  assert(result!=NULL);
  result->size=1;
  result->int_array=malloc(sizeof(unsigned int));
  assert(result->int_array!=NULL);
  *result->int_array=i;
  return result;
}

large_int* opcall(large_int* a, large_int* b, oper op, char** opname, struct timespec* ts_start, struct timespec* ts_end) {
  large_int* result;

  clock_gettime(CLOCK_REALTIME, ts_start);
  switch(op) {
    case P_UNKNOWN: printf("This operation is not known. Not executing"); return large_int_copy(a);
    case S_ADD: result=add_s(a,b); *opname=strdup("sequential add"); break;
    case P_ADD: result=add(a,b); *opname=strdup("parallel add"); break;
    case S_SUB: result=sub_s(a,b); *opname=strdup("sequential substruct"); break;
    case P_SUB: result=sub(a,b); *opname=strdup("parallel substruct"); break;
    case S_MUL: result=mult_s(a,b); *opname=strdup("sequential multiply"); break;
    case P_MUL: result=mult(a,b); *opname=strdup("parallel multiply"); break;
    case S_KMUL: result=mult_kar_s(a,b); *opname=strdup("sequential Kartsuba multiply"); break;
    case P_KMUL: omp_set_nested(KMUX_NESTING);result=mult_kar(a,b); *opname=strdup("parallel Karatsuba multiply"); break;
    case S_DIV: result=division_s(a,b); *opname=strdup("sequential division"); break;
    case P_DIV: result=division(a,b); *opname=strdup("parallel division"); break;
    case S_NDIV: result=div_Newton_s(a,b); *opname=strdup("sequential Newton division"); break;
    case P_NDIV: result=div_Newton(a,b); *opname=strdup("parallel Newton division"); break;
    case S_LSFT: result=lsft_s(a,b); *opname=strdup("sequential bitwise left shift"); break;
    case P_LSFT: result=lsft(a,b); *opname=strdup("parallel bitwise left shift"); break;
    case S_RSFT: result=rsft_s(a,b); *opname=strdup("sequential bitwise right shift"); break;
    case P_RSFT: result=rsft(a,b); *opname=strdup("parallel bitwise right shift"); break;
    case S_AND: result=bit_and_s(a,b); *opname=strdup("sequential bitwise AND"); break;
    case P_AND: result=bit_and(a,b); *opname=strdup("parallel bitwise AND"); break;
    case S_OR: result=bit_or_s(a,b); *opname=strdup("sequential bitwise OR"); break;
    case P_OR: result=bit_or(a,b); *opname=strdup("parallel bitwise OR"); break;
    case S_NOT: result=bit_not_s(a); *opname=strdup("sequential bitwise NOT"); break;
    case P_NOT: result=bit_not(a); *opname=strdup("parallel bitwise NOT"); break;
    case S_XOR: result=bit_xor_s(a,b); *opname=strdup("sequential bitwise XOR"); break;
    case P_XOR: result=bit_xor(a,b); *opname=strdup("parallel bitwise XOR"); break;
    case S_TST: result=int_large_int(test_s(a,b)); *opname=strdup("sequential bitwise TST"); break;
    case P_TST: result=int_large_int(test(a,b)); *opname=strdup("parallel bitwise TST"); break;
    default: result=large_int_copy(a); *opname=strdup("unsupported");
  }
  clock_gettime(CLOCK_REALTIME, ts_end);
  return result;
}

// Performs operation based on op enum, prints out how long it took
large_int* perform_oper(large_int* a, large_int* b, oper op) {
  struct timespec ts_start, ts_end;
  long tsec, tmsec;
  large_int* result;
  char* opname;
  double sec;
  long long nanosec;

  result=opcall(a,b,op,&opname,&ts_start,&ts_end);
  find_timediff(ts_start,ts_end,&nanosec,&sec,&tsec,&tmsec);
  printf("The operation %s took %ld.%06ld seconds (%lld nanoseconds) to execute\n",opname,tsec,tmsec,nanosec);
  free(opname);
  return result;
}

large_int* test_oper(large_int* a, large_int* b, oper op, double *t) {
  large_int* result;
  struct timespec ts_start, ts_end;
  long tsec,tmsec;
  char* opname;
  long long nanosec;

  result=opcall(a,b,op,&opname,&ts_start,&ts_end);
  find_timediff(ts_start,ts_end,&nanosec,t,&tsec,&tmsec);
  free(opname);
  return result;
}

// Verify operation with GMP
large_int* verify_oper(large_int* a, large_int* b, oper op) {
  large_int* result;
  char* opname;

  switch(op) {
    case P_UNKNOWN: printf("This operation is not known. Not executing"); return large_int_copy(a);
    case S_ADD: result=add_gmp(a,b); opname=strdup("single add"); break;
    case P_ADD: result=add_gmp(a,b); opname=strdup("parallel add"); break;
    case S_SUB: result=sub_gmp(a,b); opname=strdup("single substruct"); break;
    case P_SUB: result=sub_gmp(a,b); opname=strdup("parallel substruct"); break;
    case S_MUL: result=mult_gmp(a,b); opname=strdup("single multiply"); break;
    case P_MUL: result=mult_gmp(a,b); opname=strdup("parallel multiply"); break;
    case S_KMUL: result=mult_gmp(a,b); opname=strdup("single kartsuba multiply"); break;
    case P_KMUL: result=mult_gmp(a,b); opname=strdup("parallel karatsuba multiply"); break;
    case S_DIV: result=division_gmp(a,b); opname=strdup("single division"); break;
    case P_DIV: result=division_gmp(a,b); opname=strdup("parallel division"); break;
    case S_NDIV: result=division_gmp(a,b); opname=strdup("single Newton division"); break;
    case P_NDIV: result=division_gmp(a,b); opname=strdup("parallel Newton division"); break;
    default: result=large_int_copy(a); opname=strdup("unsupported");
  }
  return result;
}

// For testing: Performs operation on two randomly generated numbers of specified sizes
void rnum_oper(int size1, int size2, oper op) {
  large_int *a, *b, *c, *d;
  a=get_random(size1);
  b=get_random(size2);
  int verify = op < S_LSFT ? 1 : 0;

  if (debug) {
    printf("a: "); print_hex(a); printf("a: "); print_dec(a); printf("\n");
    printf("b: "); print_hex(b); printf("b: "); print_dec(b); printf("\n");
  }

  c=perform_oper(a,b,op); printf("\n");
  if (verify) d=verify_oper(a,b,op);

  if (debug) {
    printf("c:\n"); print_hex(c); printf("c:\n"); print_dec(c);
    if (verify)
      printf("d:\n"); print_hex(d); printf("d:\n"); print_dec(d);
  }

  if (verify) {
    if (compare(c,d)) {
      printf("c:\n"); print_hex(c); printf("c:\n"); print_dec(c);
      printf("d:\n"); print_hex(d); printf("d:\n"); print_dec(d);
    }
    assert(compare(c,d) == 0);
  }

  free(a->int_array);
  free(b->int_array);
  if (c) free(c->int_array);
  if (verify) free(d->int_array);
  free(a);
  free(b);
  if (c) free(c);
  if (verify) free(d);
}
