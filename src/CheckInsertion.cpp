//===- ScalarReplAggregates.cpp - Scalar Replacement of Aggregates --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This transformation implements the well known scalar replacement of
// aggregates transformation.  This xform breaks up alloca instructions of
// structure type into individual alloca instructions for
// each member (if possible).  Then, if possible, it transforms the individual
// alloca instructions into nice clean scalar SSA form.
//
// This combines an SRoA algorithm with Mem2Reg because they
// often interact, especially for C++ programs.  As such, this code
// iterates between SRoA and Mem2Reg until we run out of things to promote.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "kint"
#include <iostream>
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/ValueTracking.h"
using namespace llvm;

STATISTIC(NumInserted,  "Number of bounds check inserted");

namespace { // Begin anonymous namespace
  
struct CheckInsertion : public FunctionPass {
  static char ID; // Pass identification
  CheckInsertion() : FunctionPass(ID) { }

  // Entry point
  bool runOnFunction(Function &F);

  // getAnalysisUsage - List passes required by this pass.  We also know it
  // will not alter the CFG, so say so.
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
  }

private:
  // Add fields and helper functions for this pass here.
  void insertCheck(BinaryOperator *I);
  bool isObservable(Instruction *I);
};

} // End anonymous namespace

char CheckInsertion::ID = 0;
static RegisterPass<CheckInsertion> X("kint-check-insertion",
          "BoundsCheckInsertion pass for Kint",
          false /* does not modify the CFG */,
          false /* transformation, not just analysis */);


// Public interface to create the ScalarReplAggregates pass.
// This function is provided to you.
FunctionPass *createMyScalarReplAggregatesPass() { 
  return new CheckInsertion();
}


//===----------------------------------------------------------------------===//
//                      SKELETON FUNCTION TO BE IMPLEMENTED
//===----------------------------------------------------------------------===//
//
// Function runOnFunction:
// Entry point for the overall ScalarReplAggregates function pass.
// This function is provided to you.
bool CheckInsertion::runOnFunction(Function &F) {

  bool Changed = false;

  for (auto &BB: F) {
    for (auto &I: BB) {
      auto *BO = dyn_cast<BinaryOperator>(&I);
      if (!BO || !isObservable(BO))
        continue;
      
      Changed = true;
      insertCheck(BO);
    }
  }

  return Changed;

}

void CheckInsertion::insertCheck(BinaryOperator *I) {
  auto *M = I->getParent()->getParent()->getParent();
  auto &C = M->getContext();

  auto *Op1 = I->getOperand(0);
  auto *Op2 = I->getOperand(1);

  Type *ArgTys[3] = { Type::getInt8Ty(C), Op1->getType(), Op2->getType()};
  auto *FnTy = FunctionType::get(Type::getVoidTy(C), ArgTys, false);
  auto F = M->getOrInsertFunction("kint_bug_on", FnTy);

  auto *BOp = Constant::getIntegerValue(Type::getInt8Ty(C), APInt(8, I->getOpcode()));

  Value *Args[3] = { BOp, Op1, Op2 };
  CallInst::Create(F, Args, "", I);
}

bool CheckInsertion::isObservable(Instruction *I) {
  SmallVector<Instruction *, 16> insts;
  SmallPtrSet<Value *, 16> visited;

  insts.push_back(I);
  while (!insts.empty()) {
    auto *curr = insts.back();
    insts.pop_back();
    visited.insert(curr);
    
    if (isSafeToSpeculativelyExecute(curr))
      return true;

    for (auto *U: curr->users()) {
      if (!visited.contains(U))
        insts.push_back(cast<Instruction>(U));
    }
  }
  return false;
}




























