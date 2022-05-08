#ifndef SMTSOLVER_H
#define SMTSOLVER_H

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/raw_ostream.h>
#include <boolector.h>
#include <cstdio>

using llvm::errs;

typedef BoolectorNode *SMTExpr;

class SMTSolver {
  Btor *btor;

public:
  SMTSolver() : btor(boolector_new())
  {
    boolector_set_opt(btor, BTOR_OPT_PRETTY_PRINT, 1);
    boolector_set_opt(btor, BTOR_OPT_MODEL_GEN, 1);
  }
  ~SMTSolver()
  {
    boolector_release_all(btor);
    boolector_delete(btor);
  }

  // don't allow copy/move
  SMTSolver(const SMTSolver &) = delete;
  SMTSolver(SMTSolver &&) = delete;
  SMTSolver &operator=(const SMTSolver &) = delete;
  SMTSolver &operator=(SMTSolver &&) = delete;

  SMTExpr smt_true()
  {
    return boolector_true(btor);
  }

  SMTExpr smt_false()
  {
    return boolector_false(btor);
  }

  SMTExpr smt_neg(SMTExpr e1)
  {
    return boolector_neg(btor, e1);
  }

  SMTExpr smt_and(SMTExpr e1, SMTExpr e2)
  {
    return boolector_and(btor, e1, e2);
  }

  SMTExpr smt_or(SMTExpr e1, SMTExpr e2)
  {
    return boolector_or(btor, e1, e2);
  }

  SMTExpr smt_xor(SMTExpr e1, SMTExpr e2)
  {
    return boolector_xor(btor, e1, e2);
  }

  SMTExpr smt_add(SMTExpr e1, SMTExpr e2)
  {
    return boolector_add(btor, e1, e2);
  }

  SMTExpr smt_sadd_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_saddo(btor, e1, e2);
  }

  SMTExpr smt_uadd_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_uaddo(btor, e1, e2);
  }

  SMTExpr smt_sub(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sub(btor, e1, e2);
  }

  SMTExpr smt_ssub_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ssubo(btor, e1, e2);
  }

  SMTExpr smt_usub_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_usubo(btor, e1, e2);
  }

  SMTExpr smt_mul(SMTExpr e1, SMTExpr e2)
  {
    return boolector_mul(btor, e1, e2);
  }

  SMTExpr smt_smul_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_smulo(btor, e1, e2);
  }

  SMTExpr smt_umul_overflow(SMTExpr e1, SMTExpr e2)
  {
    return boolector_umulo(btor, e1, e2);
  }

  SMTExpr smt_udiv(SMTExpr e1, SMTExpr e2)
  {
    return boolector_udiv(btor, e1, e2);
  }

  SMTExpr smt_sdiv(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sdiv(btor, e1, e2);
  }

  SMTExpr smt_urem(SMTExpr e1, SMTExpr e2)
  {
    return boolector_urem(btor, e1, e2);
  }

  SMTExpr smt_srem(SMTExpr e1, SMTExpr e2)
  {
    return boolector_srem(btor, e1, e2);
  }

  SMTExpr smt_shl(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sll(btor, e1, e2);
  }

  SMTExpr smt_lshr(SMTExpr e1, SMTExpr e2)
  {
    return boolector_srl(btor, e1, e2);
  }

  SMTExpr smt_ashr(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sra(btor, e1, e2);
  }

  SMTExpr smt_eq(SMTExpr e1, SMTExpr e2)
  {
    return boolector_eq(btor, e1, e2);
  }

  SMTExpr smt_ne(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ne(btor, e1, e2);
  }

  SMTExpr smt_ugt(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ugt(btor, e1, e2);
  }

  SMTExpr smt_uge(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ugte(btor, e1, e2);
  }

  SMTExpr smt_ult(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ult(btor, e1, e2);
  }

  SMTExpr smt_ule(SMTExpr e1, SMTExpr e2)
  {
    return boolector_ulte(btor, e1, e2);
  }

  SMTExpr smt_sgt(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sgt(btor, e1, e2);
  }

  SMTExpr smt_sge(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sgte(btor, e1, e2);
  }

  SMTExpr smt_slt(SMTExpr e1, SMTExpr e2)
  {
    return boolector_slt(btor, e1, e2);
  }

  SMTExpr smt_sle(SMTExpr e1, SMTExpr e2)
  {
    return boolector_slte(btor, e1, e2);
  }

  unsigned smt_get_width(SMTExpr e)
  {
    return boolector_get_width(btor, e);
  }

  SMTExpr smt_zext(SMTExpr e, uint32_t width)
  {
    return boolector_uext(btor, e, width);
  }

  SMTExpr smt_sext(SMTExpr e, uint32_t width)
  {
    return boolector_sext(btor, e, width);
  }

  SMTExpr smt_slice(SMTExpr e, uint32_t upper, uint32_t lower)
  {
    return boolector_slice(btor, e, upper, lower);
  }

  // Assume 32 bit int
  SMTExpr smt_const(const llvm::APInt &val)
  {
    auto bvsort = boolector_bitvec_sort(btor, val.getBitWidth());

    llvm::SmallString<20> str;
    val.toStringUnsigned(str, 16);

    auto result = boolector_consth(btor, bvsort, str.c_str());
    boolector_release_sort(btor, bvsort);
    return result;
  }

  SMTExpr smt_cond(SMTExpr c, SMTExpr t, SMTExpr e)
  {
    return boolector_cond(btor, c, t, e);
  }

  SMTExpr smt_var(uint32_t width, std::string name)
  {
    auto bvsort = boolector_bitvec_sort(btor, width);
    auto result = boolector_var(btor, bvsort, name.c_str());
    boolector_release_sort(btor, bvsort);
    return result;
  }

  void smt_copy(SMTExpr e)
  {
    boolector_copy(btor, e);
  }

  void smt_release(SMTExpr e)
  {
    boolector_release(btor, e);
  }

  bool smt_query(SMTExpr e)
  {
    boolector_assert(btor, e);
    auto result = boolector_sat(btor);
    switch (result) {
    case BOOLECTOR_SAT:
      return true;
    case BOOLECTOR_UNSAT:
      return false;
    default:
      assert(0 && "smt_query unreachable");
    }
  }

  void smt_dump()
  {
    boolector_dump_smt2(btor, stderr);
  }

  void smt_print_model(char *fmt)
  {
    boolector_print_model(btor, fmt, stderr);
  }
};

#endif /* SMTSOLVER_H */
