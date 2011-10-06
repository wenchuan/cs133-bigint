#ifndef LARGE_INT
#define LARGE_INT

/*****************************************************
 *   API
 ****************************************************/

typedef struct large_int {
	unsigned int* int_array;
	unsigned int size;
  int sign;
}large_int;

large_int* add(large_int* first, large_int* second);
large_int* add_s(large_int* first, large_int* second);

large_int* sub(large_int* first, large_int* second);
large_int* sub_s(large_int* first, large_int* second);

large_int* mult(large_int* first, large_int* second);
large_int* mult_s(large_int* first, large_int* second);

large_int* mult_kar(large_int* first, large_int* second);
large_int* mult_kar_s(large_int* first, large_int* second);

large_int* division(large_int* first, large_int* second);
large_int* division_s(large_int* first, large_int* second);

large_int* div_Newton(large_int* A, large_int* B);
large_int* div_Newton_s(large_int* A, large_int* B);

large_int* rsft(large_int* first, large_int* second);
large_int* rsft_s(large_int* first, large_int* second);

large_int* lsft(large_int* first, large_int* second);
large_int* lsft_s(large_int* first, large_int* second);

large_int* rshift(large_int* first, int second);
large_int* rshift_s(large_int* first, int second);

large_int* lshift(large_int* first, int second);
large_int* lshift_s(large_int* first, int second);

large_int* bit_and(large_int* first, large_int* second);
large_int* bit_and_s(large_int* first, large_int* second);

large_int* bit_or(large_int* first, large_int* second);
large_int* bit_or_s(large_int* first, large_int* second);

large_int* bit_xor(large_int* first, large_int* second);
large_int* bit_xor_s(large_int* first, large_int* second);

large_int* bit_not(large_int* first);
large_int* bit_not_s(large_int* first);

int test_s(large_int* first, large_int* second);
int test(large_int* first, large_int* second);

void print_hex(large_int *li);
void print_dec(large_int *li);

large_int* get_hex(const char *filename);
large_int* get_dec(const char *filename);

/*****************************************************
 *   Constant definition
 ****************************************************/
#ifndef UINT_MAX
#define UINT_MAX	4294967295U
#endif

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16
#define uint16_t unsigned short
#endif

/****************************************************
 * Karatsuba Multiplication tunable parameters
 ****************************************************/
#define KMUX_MINLEVEL 4  // Minimum size word at which Karatsuba reverts
		         // to standard multiplication
#define KMUX_NESTING 0   // This should be set to 1 to enable openmp nesting
			 // WARNING: this will crash with openmp memory errors

/*****************************************************
 *   Internal functions and utilities
 ****************************************************/
extern int debug;

typedef enum {
  S_ADD,
  P_ADD,
  S_SUB,
  P_SUB,
  S_MUL,  // Regular multiplication
  P_MUL,
  S_KMUL, // Karatsuba multiplication with divide-conqr
  P_KMUL,
  S_DIV,
  P_DIV,
  S_NDIV, // Division by Newton method
  P_NDIV,
  S_LSFT,
  P_LSFT,
  S_RSFT,
  P_RSFT,
  S_AND,
  P_AND,
  S_OR,
  P_OR,
  S_XOR,
  P_XOR,
  S_NOT,
  P_NOT,
  S_TST,
  P_TST,
  P_UNKNOWN
} oper;

int count_bit(large_int* li);
void free_li(large_int* li);
void normalize(large_int *li);
void print(large_int *li);
int compare(large_int* first, large_int* second);
void shift_right(large_int* a, int n);
void shift_left(large_int* a, int n);
void shift_left2(large_int* a, int n);
inline void longlong2ints(uint64_t *ll, uint32_t *h, uint32_t *l);
large_int* large_int_copy(large_int* a);
large_int* int_large_int(int i);

large_int *get_random(int size);  // size in int
large_int *regenerate(large_int *li); // random its content, no malloc

large_int* add_gmp(large_int *first, large_int* second);
large_int* sub_gmp(large_int *first, large_int* second);
large_int* mult_gmp(large_int *first, large_int* second);
large_int* division_gmp(large_int *first, large_int* second);

void die(char*);
large_int* opcall(large_int* a, large_int* b, oper op, char** opname, struct timespec* ts_start, struct timespec* ts_end);
large_int* perform_oper(large_int* a, large_int* b, oper op);
large_int* test_oper(large_int* a, large_int* b, oper op, double *t);
large_int* verify_oper(large_int* a, large_int* b, oper op);
void rnum_oper(int size1, int size2, oper op);
oper str2oper(char* str);
void usage_common();

int batch_test(char* progname, int argc, char* argv[]);
int sam_test(int argc, char* argv[]);

#endif
