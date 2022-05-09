/*
gcc playground.c ../../boolector/build/lib/libboolector.a \
  ../../boolector/deps/install/lib/liblgl.a \
  ../../boolector/deps/install/lib/libbtor2parser.a -I../../boolector/src -lm
*/
#include <stdio.h>
#include "boolector.h"

int main(void) {
  Btor *btor = boolector_new ();
  boolector_set_opt(btor, BTOR_OPT_PRETTY_PRINT, 1);
  boolector_set_opt(btor, BTOR_OPT_MODEL_GEN, 1);

  BoolectorSort bvsort32 = boolector_bitvec_sort (btor, 32);
  BoolectorNode *v1000 = boolector_constd(btor, bvsort32, "1000");
  BoolectorNode *v100 = boolector_constd(btor, bvsort32, "100");
  BoolectorNode *x = boolector_var(btor, bvsort32, "x");
  BoolectorNode *arg1 = boolector_var(btor, bvsort32, "arg1");

  // sadd_overflow(x, 100)
  BoolectorNode *o = boolector_saddo(btor, x, v100);

  // true
  BoolectorNode *br34 = boolector_true(btor);
  // x == 1000
  BoolectorNode *phi34 = boolector_eq(btor, x, v1000);
  // x == 1000
  BoolectorNode *brPhi34 = boolector_and(btor, br34, phi34);

  // arg1 > 1000
  BoolectorNode *br13 = boolector_sgt(btor, arg1, v1000);
  // (arg1 > 1000) && (x == 1000)
  BoolectorNode *path3 = boolector_and(btor, brPhi34, br13);

  // !(arg1 > 1000)
  BoolectorNode *br14 = boolector_neg(btor, br13);
  // x == arg1
  BoolectorNode *phi14 = boolector_eq(btor, x, arg1);
  // !(arg1 > 1000) && (x == arg1)
  BoolectorNode *path1 = boolector_and(btor, br14, phi14);

  // (arg1 > 1000) && x == 1000 || !(arg1 > 1000) && (x == arg1)
  BoolectorNode *path = boolector_or(btor, path3, path1);

  // ((arg1 > 1000) && x == 1000 || !(arg1 > 1000) && (x == arg1)) && sadd_overflow(x, 100)
  BoolectorNode *final = boolector_and(btor, o, path);

  boolector_assert(btor, final);

  int result = boolector_sat(btor);

  if (result == BOOLECTOR_SAT) {
    printf("SAT\n");
    boolector_print_model(btor, "smt2", stderr);
  } else {
    printf("Not SAT\n");
  }
  boolector_dump_smt2(btor, stderr);
}
