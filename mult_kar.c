#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <omp.h>

#include "largeint.h"

#define ADD add_s
#define SUB sub_s

// Prototype of our main function that is private here and not in largeint.h
large_int* mult_kar_eqsize(large_int* a, large_int* b);
  
// Function that splits multiple of two large numbers into
// multiplication of numbers about same size. It does it in
// divide and conquer fashion by splitting one that is larger
// than the other in half i.e. (A1*2^N+A0)*B=A1*B*2^N+A0*B
large_int* mult_kar(large_int* a, large_int* b) {
  int n;
  large_int *c,*d; // Depending on which is larger these become either a or b
  large_int c0,c1; // Split in half large half of the a or b
  large_int *mc0,*mc1,*result;

if (debug) printf("Calling mult_kar\n");

  // Decide which number is larger
  if (a->size>b->size && a->size>2*b->size) {
    n=a->size/2;
    c0.size=n;
    c1.size=a->size-n;
    c=a;
    d=b;
  }
  else if (b->size>a->size && b->size>2*a->size) {
    n=b->size/2;
    c0.size=n;
    c1.size=b->size-n;
    c=b;
    d=a;
  }
  else {
    // Numbers close to equal in size, we call on Karatsuba algorithm
    return mult_kar_eqsize(a,b);
  }
  
  // Allocate memory and copy data for c0, c1
  c0.int_array=malloc(sizeof(unsigned int)*c0.size);
  c1.int_array=malloc(sizeof(unsigned int)*c1.size);
  if (c0.int_array==NULL || c1.int_array==NULL)
    die("memory allocation error");
  memcpy(c0.int_array,c->int_array,c0.size*sizeof(unsigned int));
  memcpy(c1.int_array,(char*)c->int_array+((size_t)c0.size*sizeof(unsigned int)),c1.size*sizeof(unsigned int));
  
  // Recursive step that can be parallelized
  #pragma omp parallel sections default (shared)
  {
   mc0=mult_kar(&c0,d);
   #pragma omp section
   mc1=mult_kar(&c1,d);
  }

  // Shift and make one number
  shift_left(mc1,n);
  result=ADD(mc1,mc0);

  // Free and Return
  free(c0.int_array);
  free(c1.int_array);
  free(mc0->int_array);
  free(mc1->int_array);
  free(mc0);
  free(mc1);
  return result;
}

// Sequential version of Karatsuba multiplication, with recursion
// It works well when numbers are about same size
large_int* mult_kar_eqsize(large_int* a, large_int* b) {
  int n;
  large_int a0,a1,b0,b1;
  large_int *a1_plus_a0,*b1_plus_b0, *a1b0_plus_a0b1;
  large_int *a1_mult_b1,*a0_mult_b0, *a1pa0_mult_b1pb0;
  large_int *temp,*result;

if (debug) printf("Calling mult_kar_eqsize\n");

  // For small numbers we use standard multiplication algorithm
  // This is the end of the recursion
  if (a->size<=KMUX_MINLEVEL || b->size<=KMUX_MINLEVEL) {
if (debug) printf("Calling mult_gmp\n");
	// return mult_gmp(a,b);
	return mult_s(a,b);
  }

  // Here we choose first if a or b are smaller. Then out of the smallest one
  // which we can call C, we find n that is <= 1/2 of it
  if (a->size>b->size) {
    n=b->size/2;
    if (2*n<b->size) n++;
  }
  else {
    n=a->size/2;
    if (2*n<a->size) n++;
  }
  a0.size=n;
  a1.size=a->size-n;
  b0.size=n;
  b1.size=b->size-n;
  // allocate arrays
  a0.int_array=malloc(sizeof(unsigned int)*a0.size);
  a1.int_array=malloc(sizeof(unsigned int)*a1.size);
  b0.int_array=malloc(sizeof(unsigned int)*b0.size);
  b1.int_array=malloc(sizeof(unsigned int)*b1.size);
  if (a0.int_array==NULL || a1.int_array==NULL || b0.int_array==NULL || b1.int_array==NULL)
    die("memory allocation error");

  // Copy data from a,b into a0,a1,b0,b1
  memcpy(a0.int_array,a->int_array,a0.size*sizeof(unsigned int));
  memcpy(a1.int_array,(char*)a->int_array+((size_t)a0.size*sizeof(unsigned int)),a1.size*sizeof(unsigned int));
  memcpy(b0.int_array,b->int_array,b0.size*sizeof(unsigned int));
  memcpy(b1.int_array,(char*)b->int_array+((size_t)b0.size*sizeof(unsigned int)),b1.size*sizeof(unsigned int));

  // Calculate a0+a1, b1+b0 - these can potentially be done in parallel to each other
  #pragma omp parallel sections default (shared)
  {
   a1_plus_a0=ADD(&a1,&a0);
   #pragma omp section
   b1_plus_b0=ADD(&b1,&b0);
  }

  // Calculate 3 intermediate products - can all be done in parallel
  #pragma omp parallel sections default (shared)
  {
if (debug) printf("d0 ");
   a1_mult_b1=mult_kar_eqsize(&a1,&b1);
if (debug) { printf("a1*b1 : "); print_hex(a1_mult_b1); }
   #pragma omp section
   {
    a0_mult_b0=mult_kar_eqsize(&a0,&b0);
if (debug) { printf("a0*b0 : "); print_hex(a0_mult_b0); } 
   }
   #pragma omp section
   {
   a1pa0_mult_b1pb0=mult_kar_eqsize(a1_plus_a0,b1_plus_b0);
if (debug) { printf("(a1+a0)*(b1+b0) : "); print_hex(a1pa0_mult_b1pb0); }
   }
  }

  // From here on things can not really be parallelizeable
  if (debug && compare(a1_mult_b1,a1pa0_mult_b1pb0)>0) {
    printf("Noticed that a1b1 > (a1+a0)(b1+b0), this should not be\n");
    temp=SUB(a1_mult_b1,a1pa0_mult_b1pb0);
    temp->sign=-1;
  }
  else {
    if (debug) printf("Calling sub -a1b1 , first_size=%d, second_size=%d\n",a1pa0_mult_b1pb0->size,a1_mult_b1->size);
    temp=SUB(a1pa0_mult_b1pb0,a1_mult_b1);
    temp->sign=1;
    if (debug) print_hex(temp);
  }
  if (debug && compare(temp,a0_mult_b0)<0) {
    if (debug) printf("Noticed that a0b0 > reminder, should not be");
    a1b0_plus_a0b1=SUB(a0_mult_b0,temp);
    if (temp->sign==-1 && debug) printf("but we're good\n");
    else { printf("we have a problem\n"); a1b0_plus_a0b1->sign=-1; }
  }
  else {
    if (debug) printf("calling sub -a0b0: ");
    a1b0_plus_a0b1=SUB(temp,a0_mult_b0);
    if (temp->sign==-1) printf("we have a problem\n");
    if (debug) print_hex(a1b0_plus_a0b1);
  }
  free(temp->int_array);
  free(temp);

  // Do some shifting around to prepare final result
  shift_left(a1b0_plus_a0b1,n);
  shift_left(a1_mult_b1,n*2);
 
  if (debug) { 
       printf("Shifted a1b0_plus_a0b1: ");
       print_hex(a1b0_plus_a0b1);
       printf("Shifted a1_mult_b1: ");
       print_hex(a1_mult_b1);
  }

  // Now we're ready to put it all together
  temp=ADD(a1_mult_b1,a1b0_plus_a0b1);
  result=ADD(temp,a0_mult_b0);

  // Free all allocated memory
  free(temp->int_array);
  free(a1_mult_b1->int_array);
  free(a0_mult_b0->int_array);
  free(a1pa0_mult_b1pb0->int_array);
  free(a1_plus_a0->int_array);
  free(b1_plus_b0->int_array);
  free(a1b0_plus_a0b1->int_array);
  free(a0.int_array);
  free(a1.int_array);
  free(b0.int_array);
  free(b1.int_array);
  free(temp);
  free(a1_mult_b1);
  free(a0_mult_b0);
  free(a1pa0_mult_b1pb0);
  free(a1_plus_a0);
  free(b1_plus_b0);
  free(a1b0_plus_a0b1);

  // And return
  return result;
}
