#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "largeint.h"

int debug = 0;

void usage_common();
void usage_std(char*);
void usage_load(char*);
int do_load(int type, int argc, char** argv);

int main(int argc, char* argv[])
{
	int size1,size2;
	oper op;
	int startarg=1;

	srand(time(NULL));

  	if (argc > 1 && strcmp(argv[1], "test") == 0)
    	   return batch_test(argv[0], argc - 1, argv+1);

	if (argc > 1 && strcmp(argv[1], "debug") == 0) {
	   debug=1;
	   startarg=2;
	}  

	if (argc >1 && !strcmp(argv[1], "load_hex"))
	   return do_load(1,argc,argv);
	if (argc >1 && !strcmp(argv[1], "load_dec"))
	   return do_load(2,argc,argv);

	if (argc!=(startarg+3)) usage_std(argv[0]);
	   
        size1=atoi(argv[startarg]);
        op=str2oper(argv[startarg+1]);
        size2=atoi(argv[startarg+2]);
        if (op==P_UNKNOWN) usage_std(argv[0]);

        rnum_oper(size1, size2, op);
        exit(0);
}

void usage_std(char* progname) {
	printf("Usage: %s [debug|load_hex|load_dec|test] <size num1> <oper> <size num2>\n",progname);
	printf("   where <size num> are number of bits in multiple of 32 bits\n");
	printf("   and <oper> is one of:\n");
	usage_common();
	printf("\n for help on how to use load and test please do:\n");
	printf("   %s load_hex\n",progname);
	printf("   %s test\n",progname);
	exit(1);
}

void usage_common() {
	printf("      add | add_s  - add numbers\n");
	printf("      sub | sub_s  - substract\n");
	printf("      mul | mul_s  - multiply\n");
	printf("     kmul | kmul_s - Karatsuba multiply\n");
	printf("      div | div_s  - divide\n");
	printf("     ndiv | ndiv_s - disivion by Netwon method\n");
	printf("     lsft | lsft_s - Bitwise left shift\n");
	printf("     rsft | rsft_s - Bitwise right shift\n");
	printf("      and | and_s  - Bitwise AND\n");
	printf("       or | or_s   - Bitwise OR\n");
	printf("      xor | xor_s  - Bitwise XOR\n");
	printf("      not | not_s  - Bitwise NOT\n");
	printf("      tst | tst_s  - Bitwise equality test\n");
}

void usage_load(char* progname) {
        printf("Usage: %s load_hex|load_dec <file1> <oper> <file2>\n",progname);
        printf("   where <file1> and <file2> are two files that contain\n");
	printf("   data for the operation to be performed.\n");
        printf("   and <oper> is one of:\n");
        usage_common();
        exit(1);
} 

int do_load(int type, int argc, char** argv) {
	oper op;
	FILE* fp;
	char* file_a = argv[2];
	char* file_b = argv[4];
	large_int *a,*b,*result;

	if (argc!=5) usage_load(argv[0]);

	op=str2oper(argv[3]);
	if (op==P_UNKNOWN) usage_load(argv[0]);
	if ((fp=fopen(file_a,"r"))==NULL) {
		printf("Unable to open file %s\n\n",file_a);
		usage_load(argv[0]);
	}
	else fclose(fp);
	if ((fp=fopen(file_b,"r"))==NULL) {
		printf("Unable to open file %s\n\n",file_b);
		usage_load(argv[0]);
	}
	else fclose(fp);
	if (type==1) {
		a=get_hex(file_a);
		b=get_hex(file_b);
		result=perform_oper(a,b,op);
		printf("Result of the operation is: ");
		print_hex(result);
	}	
	else if (type==2) {
		a=get_dec(file_a);
		b=get_dec(file_b);
		result=perform_oper(a,b,op);
		printf("Result of the operation is: ");
		print_dec(result);
	}
	else usage_load(argv[0]);

	free(a->int_array);
	free(b->int_array);
	free(result->int_array);
	free(a);
	free(b);
	free(result);
	return 0;
}
