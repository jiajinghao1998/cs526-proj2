#define DEBUG_TYPE "kint"
#include <llvm/Analysis/CFG.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <string>
#include "Constraints.h"
#include "SMTSolver.h"

using namespace llvm;

namespace {

struct SMTQuery : public FunctionPass {
  static char ID; // Pass identification
  SMTQuery() : FunctionPass(ID) {}

  bool runOnFunction(Function &);

  // getAnalysisUsage - List passes required by this pass.  We also know it
  // will not alter the CFG, so say so.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
  }

private:
  enum KINT_TYPE : unsigned {
    KINT_NONE = 0,
    KINT_OVERFLOW = 1,
    KINT_SHIFT_DIV = 2,
  };

  SmallPtrSet<CallInst *, 32> reports;

  void doCheck(CallInst *, ArrayRef<std::pair<const BasicBlock *, const BasicBlock *>>, KINT_TYPE);

  static KINT_TYPE matchKintFunc(const Function *);
  static void printReport(const CallInst *);
};

} // End anonymous namespace

char SMTQuery::ID = 0;

static RegisterPass<SMTQuery> X("kint-smt-query",
          "Boolector query pass for Kint",
          false /* does not modify the CFG */,
          false /* transformation, not just analysis */);

SMTQuery::KINT_TYPE SMTQuery::matchKintFunc(const Function *F) {
  auto name = static_cast<std::string>(F->getName());
  if (name.compare(0, 15, "__kint_overflow") == 0)
    return KINT_OVERFLOW;
  else if (name.compare(0, 16, "__kint_shift_div") == 0)
    return KINT_SHIFT_DIV;
  else
    return KINT_NONE;
}

void SMTQuery::printReport(const CallInst *CI) {
  auto *I = CI->getNextNode();

  errs() << "Possible Integer error: " << I->getModule()->getName() << "::"
    << I->getFunction()->getName();

  auto BBName = I->getParent()->getName();
  if (BBName != "")
    errs() << "::" << BBName;

  errs() << ": " << *I << '\n';
}

bool SMTQuery::runOnFunction(Function &F) {
  SmallVector<std::pair<const BasicBlock *, const BasicBlock *>, 16> backEdges;
  FindFunctionBackedges(F, backEdges);
  reports.clear();

  for (auto &BB: F) {
    for (auto &I: BB) {
      auto *CI = dyn_cast<CallInst>(&I);
      if (!CI)
        continue;

      auto type = matchKintFunc(CI->getCalledFunction());
      if (!type)
        continue;

      doCheck(CI, backEdges, type);
    }
  }

  return false;
}

void SMTQuery::doCheck(CallInst *CI,
  ArrayRef<std::pair<const BasicBlock *, const BasicBlock *>> backEdges, KINT_TYPE type) {
  auto &DL = CI->getModule()->getDataLayout();
  SMTSolver solver;
  ValueConstraint ValCon(solver, DL);
  PathConstraint PathCon(ValCon, backEdges);

  SMTExpr valExpr;

  switch (type) {
  case KINT_OVERFLOW:
    valExpr = ValCon.calcOverflowConstraint(CI);
    break;
  case KINT_SHIFT_DIV:
    valExpr = ValCon.calcShiftDivConstraint(CI);
    break;
  default:
    llvm_unreachable("unknown kint function type");
  }

  auto pcExpr = PathCon.calcConstraint(CI->getParent());
  auto expr = solver.smt_and(pcExpr, valExpr);
  solver.smt_release(pcExpr);
  solver.smt_release(valExpr);

  if (solver.smt_query(expr)) {
    if (!reports.contains(CI)) {
      reports.insert(CI);
      printReport(CI);
    }
  }
}
