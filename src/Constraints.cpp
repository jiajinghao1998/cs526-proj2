#include <llvm/ADT/SmallVector.h>
#include <llvm/Analysis/CFG.h>
#include <llvm/IR/GetElementPtrTypeIterator.h>
#include <llvm/Support/raw_ostream.h>
#include "Constraints.h"

using namespace llvm;

SMTExpr PathConstraint::calcConstraint(Instruction *I) {
  return calcConstraint(I->getParent());
}

SMTExpr PathConstraint::calcConstraint(BasicBlock *BB) {
  auto expr = BBToExpr.lookup(BB);
  if (expr) {
    solver.smt_copy(expr);
    return expr;
  }

  if (BB->isEntryBlock()) {
    expr = solver.smt_true();
    solver.smt_copy(expr);
    BBToExpr[BB] = expr;
    return expr;
  }

  expr = solver.smt_false();

  for(auto it = pred_begin(BB), eit = pred_end(BB); it != eit; ++it) {
    if (backEdgesSet.find(std::make_pair(*it, BB)) == backEdgesSet.end()) {
      // assume BB to be well-formed, i.e. predBr is not null
      auto predBr = (*it)->getTerminator();
      auto brExpr = calcBrConstraint(predBr, BB);
      auto assignExpr = calcAssignConstraint(BB, *it);
      auto andExpr = solver.smt_and(brExpr, assignExpr);
      solver.smt_release(brExpr);
      solver.smt_release(assignExpr);
      auto predExpr = calcConstraint(*it);
      auto andExpr2 = solver.smt_and(andExpr, predExpr);
      solver.smt_release(andExpr);
      solver.smt_release(predExpr);
      auto newExpr = solver.smt_or(andExpr2, expr);
      solver.smt_release(andExpr2);
      solver.smt_release(expr);
      expr = newExpr;
    }
  }

  solver.smt_copy(expr);
  BBToExpr[BB] = expr;
  return expr;
}

SMTExpr PathConstraint::calcAssignConstraint(BasicBlock *BB, BasicBlock *Pred) {
  auto expr = solver.smt_true();
  for (auto &I: *BB) {
    if (auto *PN = dyn_cast<PHINode>(&I)) {
      Value *V = PN->getIncomingValueForBlock(Pred);
      if (isa<UndefValue>(V) || !ValueConstraint::isAnalyzable(V->getType()))
        continue;
      auto incomingExpr = ValCon.calcConstraint(V);
      auto phiExpr = ValCon.calcConstraint(PN);
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
    auto expr = ValCon.calcConstraint(BI->getCondition());

    // Check BB path
    if (BI->getSuccessor(0) != BB) {
      auto newExpr = solver.smt_neg(expr);
      solver.smt_release(expr);
      expr = newExpr;
    }
    return expr;

  } else if (auto *SI = dyn_cast<SwitchInst>(I)) {
    auto condExpr = ValCon.calcConstraint(SI->getCondition());
    auto expr = solver.smt_false();
    for (auto C: SI->cases()) {
      // This case goes to our BB
      if (C.getCaseSuccessor() == BB) {
        auto caseExpr = ValCon.calcConstraint(C.getCaseValue());
        auto eqExpr = solver.smt_eq(condExpr, caseExpr);
        solver.smt_release(caseExpr);
        auto newExpr = solver.smt_or(expr, eqExpr);
        solver.smt_release(eqExpr);
        solver.smt_release(expr);
        expr = newExpr;
      }
    }

    if (SI->getDefaultDest() == BB) {
      auto defaultExpr = solver.smt_true();
      for (auto C: SI->cases()) {
        auto caseExpr = ValCon.calcConstraint(C.getCaseValue());
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

SMTExpr ValueConstraint::calcConstraint(llvm::Value *V) {
  auto expr = valueToExpr.lookup(V);
  if (expr) {
    solver.smt_copy(expr);
    return expr;
  }

  if (auto I = dyn_cast<Instruction>(V))
    expr = calcInstConstraint(I);
  else if (auto C = dyn_cast<Constant>(V))
    expr = calcConstConstraint(C);
  else
    expr = varConstraint(V);

  solver.smt_copy(expr);
  valueToExpr[V] = expr;

  return expr;
}

SMTExpr ValueConstraint::calcInstConstraint(llvm::Instruction *I) {
  if (auto BO = dyn_cast<BinaryOperator>(I))
    return calcBinOpConstraint(BO);
  else if (auto ICI = dyn_cast<ICmpInst>(I))
    return calcICmpConstraint(ICI);
  else if (auto GEPI = dyn_cast<GetElementPtrInst>(I))
    return calcGEPConstraint(GEPI);
  else if (auto TI = dyn_cast<TruncInst>(I))
    return calcTruncConstraint(TI);
  else if (auto ZEI = dyn_cast<ZExtInst>(I))
    return calcZExtConstraint(ZEI);
  else if (auto SEI = dyn_cast<SExtInst>(I))
    return calcSExtConstraint(SEI);
  else if (auto SI = dyn_cast<SelectInst>(I))
    return calcSelectConstraint(SI);
  else if (auto EVI = dyn_cast<ExtractValueInst>(I))
    return CalcExtractValueConstraint(EVI);
  else if (auto BCI = dyn_cast<BitCastInst>(I))
    return calcBitCastConstraint(BCI);
  else if (auto ITPI = dyn_cast<IntToPtrInst>(I))
    return calcIntToPtrConstraint(ITPI);
  else if (auto PTII = dyn_cast<PtrToIntInst>(I))
    return calcPtrToIntConstraint(PTII);
  else
    return varConstraint(I);
}

SMTExpr ValueConstraint::calcConstConstraint(llvm::Constant *C) {
  if (auto CI = dyn_cast<ConstantInt>(C))
    return solver.smt_const(CI->getValue());
  else if(auto CPN = dyn_cast<ConstantPointerNull>(C))
    return solver.smt_const(APInt::getNullValue(DL.getPointerSizeInBits()));
  else if(auto GEPO = dyn_cast<GEPOperator>(C))
    return calcGEPOConstraint(GEPO);
  else
    return varConstraint(C);
}

SMTExpr ValueConstraint::varConstraint(Value *V) {
  std::string name;
  raw_string_ostream oss(name);
  oss << *V;
  return solver.smt_var(DL.getTypeSizeInBits(V->getType()), oss.str());
}

SMTExpr ValueConstraint::calcBinOpConstraint(BinaryOperator *BO) {
  auto e1 = calcConstraint(BO->getOperand(0));
  auto e2 = calcConstraint(BO->getOperand(1));
  SMTExpr expr;

  switch (BO->getOpcode()) {
  case Instruction::Add:
    expr = solver.smt_add(e1, e2);
    break;
  case Instruction::Sub:
    expr = solver.smt_sub(e1, e2);
    break;
  case Instruction::Mul:
    expr = solver.smt_mul(e1, e2);
    break;
  case Instruction::UDiv:
    expr = solver.smt_udiv(e1, e2);
    break;
  case Instruction::SDiv:
    expr = solver.smt_sdiv(e1, e2);
    break;
  case Instruction::URem:
    expr = solver.smt_urem(e1, e2);
    break;
  case Instruction::SRem:
    expr = solver.smt_srem(e1, e2);
    break;
  case Instruction::Shl:
    expr = solver.smt_shl(e1, e2);
    break;
  case Instruction::LShr:
    expr = solver.smt_lshr(e1, e2);
    break;
  case Instruction::AShr:
    expr = solver.smt_ashr(e1, e2);
    break;
  case Instruction::And:
    expr = solver.smt_and(e1, e2);
    break;
  case Instruction::Or:
    expr = solver.smt_or(e1, e2);
    break;
  case Instruction::Xor:
    expr = solver.smt_xor(e1, e2);
    break;
  default:
    llvm_unreachable("Invalid binary operator");
  }

  solver.smt_release(e1);
  solver.smt_release(e2);
  return expr;
}

SMTExpr ValueConstraint::calcICmpConstraint(ICmpInst *ICI) {
  auto e1 = calcConstraint(ICI->getOperand(0));
  auto e2 = calcConstraint(ICI->getOperand(1));
  SMTExpr expr;

  switch(ICI->getPredicate()) {
  case CmpInst::ICMP_EQ:
    expr = solver.smt_eq(e1, e2);
    break;
  case CmpInst::ICMP_NE:
    expr = solver.smt_ne(e1, e2);
    break;
  case CmpInst::ICMP_UGT:
    expr = solver.smt_ugt(e1, e2);
    break;
  case CmpInst::ICMP_UGE:
    expr = solver.smt_uge(e1, e2);
    break;
  case CmpInst::ICMP_ULT:
    expr = solver.smt_ult(e1, e2);
    break;
  case CmpInst::ICMP_ULE:
    expr = solver.smt_ule(e1, e2);
    break;
  case CmpInst::ICMP_SGT:
    expr = solver.smt_sgt(e1, e2);
    break;
  case CmpInst::ICMP_SGE:
    expr = solver.smt_sge(e1, e2);
    break;
  case CmpInst::ICMP_SLT:
    expr = solver.smt_slt(e1, e2);
    break;
  case CmpInst::ICMP_SLE:
    expr = solver.smt_sle(e1, e2);
    break;
  default:
    llvm_unreachable("Invalid ICmp predicate");
  }

  solver.smt_release(e1);
  solver.smt_release(e2);
  return expr;
}

SMTExpr ValueConstraint::calcGEPConstraint(GetElementPtrInst *GEPI) {
  return calcGEPOConstraint(cast<GEPOperator>(GEPI));
}


SMTExpr ValueConstraint::calcGEPOConstraint(GEPOperator *GEPO) {
  auto base = calcConstraint(GEPO->getPointerOperand());
  auto ptrSize = DL.getPointerSizeInBits();
  auto ceOff = APInt::getNullValue(ptrSize);

  errs() << *GEPO << "\n";
  auto it = gep_type_begin(GEPO);
  for (int i = 1; i < GEPO->getNumOperands(); i++) {
    Value *V = GEPO->getOperand(i);
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
      auto ET = AU->getElementType();
      auto elemSize = APInt(ptrSize, DL.getTypeAllocSize(ET));
      auto elemSizeExpr = solver.smt_const(elemSize);

      auto idxExpr = calcConstraint(V);
      unsigned idxSize = solver.smt_get_width(idxExpr);

      // Sometimes a 64-bit GEP's index is 32-bit.
      // Reference: https://github.com/CRYPTOlab/kint/blob/c3402fa03ff76657045ca564a96176e356fa0e7a/src/ValueGen.cc#L198
      if (idxSize != ptrSize) {
        errs() << "calcGEPConstraint: diff size\n";
        SMTExpr Tmp;
        if (idxSize < ptrSize)
          Tmp = solver.smt_sext(idxExpr, ptrSize - idxSize);
        else
          Tmp = solver.smt_slice(idxExpr, ptrSize - 1, 0);
        solver.smt_release(idxExpr);
        idxExpr = Tmp;
      }
      auto varOffExpr = solver.smt_mul(idxExpr, elemSizeExpr);
      solver.smt_release(idxExpr);
      solver.smt_release(elemSizeExpr);
      auto newbase = solver.smt_add(base, varOffExpr);
      solver.smt_release(base);
      solver.smt_release(varOffExpr);
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

  auto ceOffExpr = solver.smt_const(ceOff);
  auto newBase = solver.smt_add(base, ceOffExpr);
  solver.smt_release(base);
  base = newBase;
  return base;
}

SMTExpr ValueConstraint::calcTruncConstraint(TruncInst *TI) {
  auto width = DL.getTypeSizeInBits(TI->getDestTy());
  auto e = calcConstraint(TI->getOperand(0));
  auto expr = solver.smt_slice(e, width - 1, 0);
  solver.smt_release(e);
  return expr;
}

SMTExpr ValueConstraint::calcZExtConstraint(ZExtInst *ZEI) {
  auto DWidth = DL.getTypeSizeInBits(ZEI->getDestTy());
  auto SWidth = DL.getTypeSizeInBits(ZEI->getSrcTy());
  auto e = calcConstraint(ZEI->getOperand(0));
  auto expr = solver.smt_zext(e, DWidth - SWidth);
  solver.smt_release(e);
  return expr;
}

SMTExpr ValueConstraint::calcSExtConstraint(SExtInst *SEI) {
  auto DWidth = DL.getTypeSizeInBits(SEI->getDestTy());
  auto SWidth = DL.getTypeSizeInBits(SEI->getSrcTy());
  auto e = calcConstraint(SEI->getOperand(0));
  auto expr = solver.smt_sext(e, DWidth - SWidth);
  solver.smt_release(e);
  return expr;
}

SMTExpr ValueConstraint::calcSelectConstraint(SelectInst *SI) {
  auto condExpr = calcConstraint(SI->getCondition());
  auto trueExpr = calcConstraint(SI->getTrueValue());
  auto falseExpr = calcConstraint(SI->getFalseValue());
  auto expr = solver.smt_cond(condExpr, trueExpr, falseExpr);
  solver.smt_release(condExpr);
  solver.smt_release(trueExpr);
  solver.smt_release(falseExpr);
  return expr;
}

SMTExpr ValueConstraint::CalcExtractValueConstraint(ExtractValueInst *EVI) {
  errs() << "CalcExtractValueConstraint: " << *EVI << '\n';
  return varConstraint(EVI);
}

SMTExpr ValueConstraint::calcBitCastConstraint(BitCastInst *BCI) {
  auto V = BCI->getOperand(0);

  // TODO: BCI->getOperand(0) might be float?
  if (!isAnalyzable(V->getType()))
    return varConstraint(BCI);

  return calcConstraint(V);
}

SMTExpr ValueConstraint::calcIntToPtrConstraint(IntToPtrInst *ITPI) {
  auto DWidth = DL.getTypeSizeInBits(ITPI->getDestTy());
  auto SWidth = DL.getTypeSizeInBits(ITPI->getSrcTy());
  auto e = calcConstraint(ITPI->getOperand(0));
  SMTExpr expr;

  if (ITPI->isNoopCast(DL))
    return e;

  assert(DWidth != SWidth);

  expr = DWidth < SWidth ? solver.smt_slice(e, DWidth - 1, 0) :
                           solver.smt_zext(e, DWidth - SWidth);
  solver.smt_release(e);
  return expr;
}

SMTExpr ValueConstraint::calcPtrToIntConstraint(PtrToIntInst *PTII) {
  auto DWidth = DL.getTypeSizeInBits(PTII->getDestTy());
  auto SWidth = DL.getTypeSizeInBits(PTII->getSrcTy());
  auto e = calcConstraint(PTII->getOperand(0));
  SMTExpr expr;

  if (PTII->isNoopCast(DL))
    return e;

  assert(DWidth != SWidth);

  expr = DWidth < SWidth ? solver.smt_slice(e, DWidth - 1, 0) :
                           solver.smt_zext(e, DWidth - SWidth);
  solver.smt_release(e);
  return expr;
}

SMTExpr ValueConstraint::calcBoundCheckConstraint(CallInst *CI) {
  auto opcode = cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue();
  auto e1 = calcConstraint(CI->getArgOperand(1));
  auto e2 = calcConstraint(CI->getArgOperand(2));
  auto nsw = static_cast<bool>(cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue());
  SMTExpr expr;

  switch (opcode) {
  case Instruction::Add:
    expr = nsw ? solver.smt_sadd_overflow(e1, e2) : solver.smt_uadd_overflow(e1, e2);
    break;
  case Instruction::Sub:
    expr = nsw ? solver.smt_ssub_overflow(e1, e2) : solver.smt_usub_overflow(e1, e2);
    break;
  case Instruction::Mul:
    expr = nsw ? solver.smt_smul_overflow(e1, e2) : solver.smt_umul_overflow(e1, e2);
    break;
  default:
    llvm_unreachable("unsupported intop");
  }

  solver.smt_release(e1);
  solver.smt_release(e2);

  return expr;
}

// Ref: https://github.com/CRYPTOlab/kint/blob/master/src/ValueGen.cc#L298
bool ValueConstraint::isAnalyzable(const Type *Ty) {
	return Ty->isIntegerTy()
		|| Ty->isPointerTy()
		|| Ty->isFunctionTy();
}
