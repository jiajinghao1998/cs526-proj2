#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include "SMTSolver.h"


class PathConstraint {
  ValueConstraint &ValCon;
  SMTSolver &solver;
  llvm::DenseMap<BasicBlock *, SMTExpr> BBToExpr;

  SMTExpr calcConstraint(BasicBlock *BB, const std::set<std::pair<BasicBlock *, BasicBlock *>> backEdgesSet);
  SMTExpr calcAssignConstraint(BasicBlock *BB, BasicBlock *Pred);
  SMTExpr calcBrConstraint(Instruction *I, BasicBlock *BB);

public:
  PathConstraint(ValueConstraint &VC) : ValCon(VC), solver(VC.solver) {}
  ~PathConstraint() = default;

  // don't allow copy/move
  PathConstraint(const PathConstraint &) = delete;
  PathConstraint(PathConstraint &&) = delete;
  PathConstraint &operator=(const PathConstraint &) = delete;
  PathConstraint &operator=(PathConstraint &&) = delete;

  SMTExpr calcConstraint(Instruction *I, SmallVector<std::pair<BasicBlock *, BasicBlock *>, 16> backEdges);
};


class ValueConstraint {
  SMTSolver &solver;
  llvm::DataLayout &DL;
  llvm::DenseMap<Value *, SMTExpr> valueToExpr;

  SMTExpr calcInstConstraint(llvm::Instruction *);
  SMTExpr calcConstConstraint(llvm::Constant *);
  SMTExpr varConstraint(value *);
  SMTExpr calcBinOpConstraint(BinaryOperator *);
  SMTExpr calcICmpConstraint(ICmpInst *);
  SMTExpr calcGEPConstraint(GetElementPtrInst *);
  SMTExpr calcGEPOConstraint(GEPOperator *);
  SMTExpr calcTruncConstraint(TruncInst *);
  SMTExpr calcZExtConstraint(ZExtInst *);
  SMTExpr calcSExtConstraint(SExtInst *);
  SMTExpr calcSelectConstraint(SelectInst *);
  SMTExpr CalcExtractValueConstraint(ExtractValueInst *);
  SMTExpr calcBitCastConstraint(BitCastInst *);
  SMTExpr calcIntToPtrConstraint(IntToPtrInst *);
  SMTExpr calcPtrToIntConstraint(PtrToIntInst *);

public:
  ValueConstraint(SMTSolver &solver, llvm::DataLayout &DL) : solver(solver),
      DL(DL) {}
  ~ValueConstraint() = default;

  // don't allow copy/move
  ValueConstraint(const ValueConstraint &) = delete;
  ValueConstraint(ValueConstraint &&) = delete;
  ValueConstraint &operator=(const ValueConstraint &) = delete;
  ValueConstraint &operator=(ValueConstraint &&) = delete;

  SMTExpr calcConstraint(llvm::Value *);

  friend class PathConstraint;
}
