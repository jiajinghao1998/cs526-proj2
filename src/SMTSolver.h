#include "boolector.h"

typedef BoolectorNode *SMTExpr;

class SMTSolver {
  Btor *btor

public:
  SMTSolver() : btor(boolector_new()) {}
  ~SMTSolver()
  {
    boolector_delete(btor);
  }

  SMTExpr smt_true()
  {
    return boolector_true(btor);
  }

  SMTExpr smt_false()
  {
    return boolector_false(btor);
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

  SMTExpr smt_sub(SMTExpr e1, SMTExpr e2)
  {
    return boolector_sub(btor, e1, e2);
  }

  SMTExpr smt_mul(SMTExpr e1, SMTExpr e2)
  {
    return boolector_mul(btor, e1, e2);
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

  SMTExpr smt_sext(SMTExpr e, uint32_t width)
  {
    return boolector_sext(btor, e, width);
  }

  SMTExpr smt_slice(SMTExpr e, uint32_t upper, uint32_t lower)
  {
    return boolector_slice(btor, e, upper, lower);
  }

  // Assume 32 bit int
  SMTExpr smt_const(llvm::APInt val)
  {
    auto bvsort = boolector_bitvec_sort(btor, val.getBitWidth());

    llvm::SmallString<20> str;
    if (val.isSignedIntN(val.getBitWidth()))
      val.toStringSigned(str, 16);
    else
      val.toStringUnSigned(str, 16);

    auto result = boolector_consth(btor, bvsort, str.c_str());
    boolector_release_sort(btor, bvsort);
    return result;
  }

  void smt_release(SMTExpr e)
  {
    boolector_release(btor, e);
  }
}
