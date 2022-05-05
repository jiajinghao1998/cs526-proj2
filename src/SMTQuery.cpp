#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>
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
  static bool isBoundCheckFunc(const Function *);
};

} // End anonymous namespace

char SMTQuery::ID = 0;

static RegisterPass<SMTQuery> X("kint-smt-query",
          "Boolector query pass for Kint",
          false /* does not modify the CFG */,
          false /* transformation, not just analysis */);

bool SMTQuery::isBoundCheckFunc(const Function *F) {
  return std::string(F->getName()).compare(0, 11, "kint_bug_on") == 0;
}

bool SMTQuery::runOnFunction(Function &F) {
  for (auto &BB: F) {
    for (auto &I: BB) {
      auto *CI = dyn_cast<CallInst>(&I);
      if (!CI || !isBoundCheckFunc(CI->getCalledFunction()))
        continue;
      errs() << *CI << '\n';
    }
  }
  return false;
}
