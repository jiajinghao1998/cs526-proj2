#include <llvm/Pass.h>
#include "Constraints.h"
#include "SMTSolver.h"

using namespace llvm;

class SMTQuery : FunctionPass {

public:
  bool runOnFunction(Function &);
};

bool SMTQuery::runOnFunction(Function &F) {
  for (auto &BB: F) {
    for (auto &I: F) {
      if (auto CI = dyn_cast<CallInst>(&I)) {
        errs() << *CI->getCalledFunction() << '\n';
      }
    }
  }
  return false;
}
