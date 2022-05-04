#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/CFG.h>
#include <set>
#include "PathConstraint.h"

using namespace llvm;

SMTExpr PathConstraint::calcConstraint(Instruction *I,
  SmallVector<std::pair<BasicBlock *, BasicBlock *>, 16> backEdges) {
  std::set<std::pair<BasicBlock *, BasicBlock *>> backEdgesSet(backEdges.begin(), backEdges.end())
  return calcConstraint(I->getParent(), backEdgesSet);
}

SMTExpr PathConstraint::calcConstraint(BasicBlock *BB,
  const std::set<std::pair<BasicBlock *, BasicBlock *>> backEdgesSet) {
  if (BB->isEntryBlock())
    return solver.smt_true();

  auto expr = solver.smt_false();

  for(auto it = pred_begin(BB), eit = pred_end(BB); it != eit; ++it) {
    if (backEdgesSet.find(std::make_pair(&*it, BB)) == backEdgesSet.end()) {
      // assume BB to be well-formed, i.e. predBr is not null
      auto predBr = it->getTerminator();
      auto brExpr = calcBrConstraint(predBr, BB);
      auto assignExpr = calcAssignConstraint(BB, &*it);
      auto andExpr = solver.smt_and(brExpr, assignExpr);
      solver.smt_release(brExpr);
      solver.smt_release(assignExpr);
      auto predExpr = calcConstraint(&*it, backEdgesSet);
      auto andExpr2 = solver.smt_and(andExpr, predExpr);
      solver.smt_release(andExpr);
      solver.smt_release(predExpr);
      auto newExpr = solver.smt_or(andExpr2, expr);
      solver.smt_release(andExpr2);
      solver.smt_release(expr);
      expr = newExpr;
    }
  }
}

SMTExpr PathConstraint::calcAssignConstraint(BasicBlock *BB, BasicBlock *Pred) {
  auto expr = solver.smt_true();
  for (auto &I: *BB) {
    if (auto *PN = dyn_cast<PHINode>(I)) {
      Value *V = getIncomingValueForBlock(Pred);
      if (isa<UndefValue>(V))
        continue;
      auto incomingExpr = ValueConstraint::calcConstraint();
      auto phiExpr = ValueConstraint::calcConstraint(PN);
      auto eqExpr = solver.smt_eq(incomingExpr, phiExpr);
      solver.smt_release(incomingExpr);
      solver.smt_release(phiExpr);
      auto newExpr = solver.smt_and(expr, eqExpr);
      solver.smt_release(expr);
      solver.smt_release(eqExpr);
      expr = newExpr;
    }
  }
  return expr;
}

SMTExpr PathConstraint::calcBrConstraint(Instruction *I, BasicBlock *BB) {
  if (auto *BI = dyn_cast<BranchInst>(I)) {
    auto expr = ValueConstraint::calcConstraint(BI->getCondition());

    // Check BB path
    if (BI->getSuccessor(0) != BB) {
      auto newExpr = solver.smt_not(expr);
      solver.smt_release(expr);
      expr = newExpr;
    }
    return expr;

  } else if (auto *SI = dyn_cast<SwitchInst>(I)) {
    auto condExpr = ValueConstraint::calcConstraint(SI->getCondition());
    auto expr = solver.smt_false();
    for (auto C: SI->cases()) {
      // This case goes to our BB
      if (C->getCaseSuccessor() == BB) {
        auto caseExpr = ValueConstraint::calcConstraint(C->getCaseValue());
        auto eqExpr = solver.smt_eq(condExpr, caseExpr);
        solver.smt_release(caseExpr);
        auto newExpr = smt_or(expr, eqExpr);
        solver.smt_release(eqExpr);
        solver.smt_release(expr);
        expr = newExpr;
      }
    }

    if (I->getDefaultDest() == BB) {
      auto defaultExpr = solver.smt_true();
      for (auto C: SI->cases()) {
        auto caseExpr = ValueConstraint::calcConstraint(C->getCaseValue());
        auto neExpr = solver.smt_ne(condExpr, caseExpr);
        solver.smt_release(caseExpr);
        auto newExpr = solver.smt_and(defaultExpr, neExpr);
        solver.smt_release(neExpr);
        solver.smt_release(defaultExpr);
        defaultExpr = newExpr;
      }
      auto newExpr = solver.smt_or(defaultExpr, expr);
      solver.smt_release(expr);
      solver.smt_release(defaultExpr);
      expr = newExpr;
    }
    return expr;

  } else if (auto *IBI = dyn_cast<IndirectBrInst>(I)) {
    return solver.smt_true();
  } else if (auto *II = dyn_cast<InvokeInst>(I)) {
    return solver.smt_true();
  }

  llvm_unreachable("Invalid terminating inst");
}

SMTExpr ValueConstraint::calcConstraint(llvm::Value *) {
  // TODO
}

SMTExpr ValueConstraint::calcInstConstraint(llvm::Value *) {
  // TODO
}

SMTExpr ValueConstraint::calcConstConstraint(llvm::Value *) {
  // TODO
}

SMTExpr ValueConstraint::calcBinOpConstraint(BinaryOperator *BO) {
  auto e1 = calcConstraint(BO->getOperand(0));
  auto e2 = calcConstraint(BO->getOperand(1));

  switch (BO->getOpcode()) {
  case Instruction::Add:
    return solver.smt_add(e1, e2);
  case Instruction::Sub:
    return solver.smt_sub(e1, e2);
  case Instruction::Mul:
    return solver.smt_mul(e1, e2);
  case Instruction::UDiv:
    return solver.smt_udiv(e1, e2);
  case Instruction::SDiv:
    return solver.smt_sdiv(e1, e2);
  case Instruction::URem:
    return solver.smt_urem(e1, e2);
  case Instruction::SRem:
    return solver.smt_srem(e1, e2);
  case Instruction::Shl:
    return solver.smt_shl(e1, e2);
  case Instruction::LShr:
    return solver.smt_lshr(e1, e2);
  case Instruction::AShr:
    return solver.smt_ashr(e1, e2);
  case Instruction::And:
    return solver.smt_and(e1, e2);
  case Instruction::Or:
    return solver.smt_or(e1, e2);
  case Instruction::Xor:
    return solver.smt_xor(e1, e2);
  default:
    llvm_unreachable("Invalid binary operator");
  }
}

SMTExpr ValueConstraint::calcICmpConstraint(ICmpInst *ICI) {
  auto e1 = calcConstraint(ICI->getOperand(0));
  auto e2 = calcConstraint(ICI->getOperand(1));

  switch(ICI->getPredicate()) {
  case CmpInst::ICMP_EQ:
    return solver.smt_eq(e1, e2);
  case CmpInst::ICMP_NE:
    return solver.smt_ne(e1, e2);
  case CmpInst::ICMP_UGT:
    return solver.smt_ugt(e1, e2);
  case CmpInst::ICMP_UGE:
    return solver.smt_uge(e1, e2);
  case CmpInst::ICMP_ULT:
    return solver.smt_ult(e1, e2);
  case CmpInst::ICMP_ULE:
    return solver.smt_ule(e1, e2);
  case CmpInst::ICMP_SGT:
    return solver.smt_sgt(e1, e2);
  case CmpInst::ICMP_SGE:
    return solver.smt_sge(e1, e2);
  case CmpInst::ICMP_SLT:
    return solver.smt_slt(e1, e2);
  case CmpInst::ICMP_SLE:
    return solver.smt_sle(e1, e2);
  default:
    llvm_unreachable("Invalid ICmp predicate");
  }
}

SMTExpr ValueConstraint::calcGEPConstraint(GetElementPtrInst *GEPI) {
  auto base = GEPI->getPointerOperand();
  auto &DL = GEPI->getParent()->getParent()->getParent()->getDataLayout();
  auto ptrSize = DL.getPointerSizeInBits();
  auto ceOff = APInt::getNullValue(ptrSize);

  errs() << *GEPI << "\n";
  auto it = gep_type_begin(GEPI);
  for (int i = 1; i < GEPI->getNumOperands(); i++) {
    Value *V = GEPI->getOperand(i);
    errs() << *V << " " << *(it.getIndexedType()) << '\n';
    if (ConstantInt *CI = dyn_cast<ConstantInt>(V)) {
      if (!CI->isZero()) {
        auto *ST = dyn_cast<StructType>(it.getIndexedType());
        if (i != 1 && ST) { // struct index
          errs() << "struct" << '\n';
          errs() << *ST << '\n';
          ceOff += DL.getStructLayout(ST)->getElementOffset(CI->getZExtValue());
        } else { // array index
          Type *ET;
          if (i == 1) {
            ET = it.getIndexedType();
          } else {
            auto AU = dyn_cast<ArrayType>(it.getIndexedType());
            assert(AU);
            ET = AU->getElementType();
          }
          errs() << "array" << '\n';
          auto elemSize = APInt(ptrSize, DL.getTypeAllocSize(ET));
          ceOff += elemSize * CI->getValue().sextOrTrunc(ptrSize);
        }
      }
    } else {//variable
      auto AU = dyn_cast<ArrayType>(it.getIndexedType());
      assert(AU);
      ET = AU->getElementType();
      auto elemSize = APInt(ptrSize, DL.getTypeAllocSize(ET));
      auto elemSizeExpr = SMT.smt_const(elemSize);

      auto idxExpr = calcConstraint(V);
      unsigned IdxSize = SMT.smt_get_width(idxExpr);

      // Sometimes a 64-bit GEP's index is 32-bit.
      // Reference: https://github.com/CRYPTOlab/kint/blob/c3402fa03ff76657045ca564a96176e356fa0e7a/src/ValueGen.cc#L198
      if (IdxSize != PtrSize) {
        errs() << "calcGEPConstraint: diff size\n";
        SMTExpr Tmp;
        if (IdxSize < PtrSize)
          Tmp = solver.sign_sext(idxExpr, PtrSize - IdxSize);
        else
          Tmp = solver.extract(idxExpr, PtrSize - 1, 0);
        solver.release(idxExpr);
        idxExpr = Tmp;
      }
      auto varOffExpr = SMT.smt_mul(idxExpr, elemSizeExpr);
      SMT.release(idxExpr);
      SMT.release(elemSizeExpr);
      auto newbase = SMT.smt_add(base, varOffExpr);
      SMT.release(base);
      SMT.release(varOffExpr);
      base = newbase;
    }

    // Increment iterator
    if (i != 1) {
      ++it;
    }
  }

  errs() << ceOff << '\n';
  if (!ceOff)
    return base;

  auto ceOffExpr = SMT.smt_const(ceOff);
  auto newBase = auto newbase = SMT.smt_add(base, ceOffExpr);
  SMT.release(base);
  base = newBase;
  return base;
}
