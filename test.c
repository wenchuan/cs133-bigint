#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>

#include "largeint.h"

double test_op_sz_sz(oper op, int fst, int snd, int round) {
  printf("test_op_sz_sz: op=%d, fst=%d, snd=%d, round=%d\n", op, fst, snd, round);
  double res = 0.0, t;
  int r;
  int verify = op < S_LSFT ? 1 : 0;
  large_int *a, *b, *c, *d;
  a = get_random(fst);
  b = get_random(snd);

  for (r = 0; r < round; r++) {
    regenerate(a); regenerate(b);
    c = test_oper(a, b, op, &t);
    res += t;

    if (verify) {
      d = verify_oper(a, b, op);
      if (compare(c,d)) {
        printf("a: "); print_dec(a); printf("\n");
        printf("b: "); print_dec(b); printf("\n");
        printf("c:\n"); print_dec(c);
        printf("d:\n"); print_dec(d);
      }
      assert(compare(c,d) == 0);
      free(d->int_array);
      free(d);
    }

    if (c) {
      free(c->int_array);
      free(c);
    }
  }
  res /= round;
  return res;
}

void test_op(char *opstr, oper op, int core_num, int round,
    int fst_sz_start, int fst_sz_end, int fst_sz_step,
    int snd_sz_start, int snd_sz_end, int snd_sz_step) {

  char namebuf[100];
  int i, j;
  double t;

  printf("Test %s on %d cores with %d round each point: %d--%d, %d; %d--%d, %d;\n",
      opstr, core_num, round, fst_sz_start, fst_sz_end, fst_sz_step,
      snd_sz_start, snd_sz_end, snd_sz_step);

  omp_set_num_threads(core_num);

  sprintf(namebuf, "%s.%d_core.%d_round.%d__%d__%d.%d__%d__%d.csv",
      opstr, core_num, round,
      fst_sz_start, fst_sz_step, fst_sz_end,
      snd_sz_start, snd_sz_step, snd_sz_end);

  FILE *fp = fopen(namebuf, "w");
  assert(fp);
  fprintf(fp, "fst_sz,snd_sz,time\n");
  for (i = fst_sz_start; i <= fst_sz_end; i += fst_sz_step) {
    for (j = snd_sz_start; j <= snd_sz_end; j += snd_sz_step) {
      t = test_op_sz_sz(op, i, j, round);
      printf("%d,\t%d,\t%f\n", i, j, t);
      fprintf(fp, "%d,%d,%f\n", i, j, t);
    }
  }
  fclose(fp);
}

int batch_test(char* prog_name, int argc, char *argv[]) {
  if (argc != 10) {
    printf("Usage: %s test <oper> <num of cores> <num of rounds>\n", prog_name);
    printf("       <int> <int> <int> range of fst input\n");
    printf("       <int> <int> <int> range of snd input\n\n");
    printf("       range is in form of <start> <end> <step>\n");
    printf("       say range is <0> <100> <10>\n");
    printf("       I'll test at point 0, 10, 20, 30, .. 100\n");
    printf("       11 points\n");
    printf("       it's for (i = fst_sz_start; i <= fst_sz_end; i += fst_sz_step)\n");
    printf("       If both operands is in form of range, then I'll test\n");
    printf("       every combination, and output to CSV file\n\n");
    printf("   and <oper> is one of:\n");
    usage_common();
    printf("\nExample test:\n");
    printf("   %s test add_s 4 10 10 110 20 100 1000 100\n", prog_name);
    return 0;
  }
  oper op = str2oper(argv[1]);
  int fst_sz_start, fst_sz_end, fst_sz_step;
  int snd_sz_start, snd_sz_end, snd_sz_step;
  int core_num = atoi(argv[2]);
  int round = atoi(argv[3]);
  fst_sz_start = atoi(argv[4]);
  fst_sz_end = atoi(argv[5]);
  fst_sz_step = atoi(argv[6]);
  snd_sz_start = atoi(argv[7]);
  snd_sz_end = atoi(argv[8]);
  snd_sz_step = atoi(argv[9]);
  char *opstr = argv[1];

  test_op(opstr, op, core_num, round, fst_sz_start, fst_sz_end, fst_sz_step,
    snd_sz_start, snd_sz_end, snd_sz_step);
  return 0;
}
