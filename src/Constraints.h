#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Type.h>
#include <set>
#include "SMTSolver.h"

class ValueConstraint {
  SMTSolver &solver;
  const llvm::DataLayout &DL;
  llvm::DenseMap<llvm::Value *, SMTExpr> valueToExpr;

  SMTExpr calcInstConstraint(llvm::Instruction *);
  SMTExpr calcConstConstraint(llvm::Constant *);
  SMTExpr varConstraint(llvm::Value *);
  SMTExpr calcBinOpConstraint(llvm::BinaryOperator *);
  SMTExpr calcICmpConstraint(llvm::ICmpInst *);
  SMTExpr calcGEPConstraint(llvm::GetElementPtrInst *);
  SMTExpr calcGEPOConstraint(llvm::GEPOperator *);
  SMTExpr calcTruncConstraint(llvm::TruncInst *);
  SMTExpr calcZExtConstraint(llvm::ZExtInst *);
  SMTExpr calcSExtConstraint(llvm::SExtInst *);
  SMTExpr calcSelectConstraint(llvm::SelectInst *);
  SMTExpr CalcExtractValueConstraint(llvm::ExtractValueInst *);
  SMTExpr calcBitCastConstraint(llvm::BitCastInst *);
  SMTExpr calcIntToPtrConstraint(llvm::IntToPtrInst *);
  SMTExpr calcPtrToIntConstraint(llvm::PtrToIntInst *);

  static bool isAnalyzable(const llvm::Type *);

public:
  ValueConstraint(SMTSolver &solver, const llvm::DataLayout &DL) : solver(solver), DL(DL) {}
  ~ValueConstraint() = default;

  // don't allow copy/move
  ValueConstraint(const ValueConstraint &) = delete;
  ValueConstraint(ValueConstraint &&) = delete;
  ValueConstraint &operator=(const ValueConstraint &) = delete;
  ValueConstraint &operator=(ValueConstraint &&) = delete;

  SMTExpr calcConstraint(llvm::Value *);
  SMTExpr calcBoundCheckConstraint(llvm::CallInst *);

  friend class PathConstraint;
};

class PathConstraint {
  ValueConstraint &ValCon;
  SMTSolver &solver;
  llvm::DenseMap<llvm::BasicBlock *, SMTExpr> BBToExpr;
  const std::set<std::pair<const llvm::BasicBlock *, const llvm::BasicBlock *>> backEdgesSet;

  SMTExpr calcAssignConstraint(llvm::BasicBlock *BB, llvm::BasicBlock *Pred);
  SMTExpr calcBrConstraint(llvm::Instruction *I, llvm::BasicBlock *BB);

public:
  PathConstraint(ValueConstraint &VC,
    llvm::ArrayRef<std::pair<const llvm::BasicBlock *, const llvm::BasicBlock *>> BE) :
    ValCon(VC), solver(VC.solver), backEdgesSet(BE.begin(), BE.end()) {}
  ~PathConstraint() = default;

  // don't allow copy/move
  PathConstraint(const PathConstraint &) = delete;
  PathConstraint(PathConstraint &&) = delete;
  PathConstraint &operator=(const PathConstraint &) = delete;
  PathConstraint &operator=(PathConstraint &&) = delete;

  SMTExpr calcConstraint(llvm::Instruction *I);
  SMTExpr calcConstraint(llvm::BasicBlock *BB);
};

#endif /* CONSTRAINTS_H */
