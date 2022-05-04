#include <llvm/IR/Instruction.h>
#include <llvm/IR/BasicBlock.h>
#include "SMTSolver.h"


class PathConstraint {
  ValueConstraint &ValCon;
  SMTSolver &solver;
public:
  PathConstraint(ValueConstraint &VC) : ValCon(VC), solver(VC.solver) {}

  SMTExpr calcConstraint(Instruction *I, SmallVector<std::pair<BasicBlock *, BasicBlock *>, 16> backEdges);
  SMTExpr calcConstraint(BasicBlock *BB, const std::set<std::pair<BasicBlock *, BasicBlock *>> backEdgesSet);
  SMTExpr calcAssignConstraint(BasicBlock *BB, BasicBlock *Pred);
  SMTExpr calcBrConstraint(Instruction *I, BasicBlock *BB);
};


class ValueConstraint {
  SMTSolver &solver;
public:
  ValueConstraint(SMTSolver &solver) : solver(solver) {}

  SMTExpr calcConstraint(llvm::Value *);

  friend class PathConstraint;
}
