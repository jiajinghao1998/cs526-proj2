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
  SmallPtrSet<CallInst *, 32> reports;

  void doCheck(CallInst *, ArrayRef<std::pair<const BasicBlock *, const BasicBlock *>>);

  static bool isBoundCheckFunc(const Function *);
  static void printReport(const CallInst *);
};

} // End anonymous namespace

char SMTQuery::ID = 0;

static RegisterPass<SMTQuery> X("kint-smt-query",
          "Boolector query pass for Kint",
          false /* does not modify the CFG */,
          false /* transformation, not just analysis */);

bool SMTQuery::isBoundCheckFunc(const Function *F) {
  return static_cast<std::string>(F->getName()).compare(0, 11, "kint_bug_on") == 0;
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
      if (!CI || !isBoundCheckFunc(CI->getCalledFunction()))
        continue;
      doCheck(CI, backEdges);
    }
  }

  for (auto P: reports)
    printReport(&*P);

  return false;
}

void SMTQuery::doCheck(CallInst *CI, ArrayRef<std::pair<const BasicBlock *, const BasicBlock *>> backEdges) {
  auto &DL = CI->getModule()->getDataLayout();
  SMTSolver solver;
  ValueConstraint ValCon(solver, DL);
  PathConstraint PathCon(ValCon, backEdges);

  auto pcExpr = PathCon.calcConstraint(CI->getParent());
  auto valExpr = ValCon.calcBoundCheckConstraint(CI);
  auto expr = solver.smt_and(pcExpr, valExpr);
  solver.smt_release(pcExpr);
  solver.smt_release(valExpr);
  if (solver.smt_query(expr)) {
    reports.insert(CI);
  }
}
