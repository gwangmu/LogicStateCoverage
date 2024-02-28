/*
 * lscov - logic state instrumtation
 * ---------------------------------
 * 
 * Mostly based on AFL. (https://github.com/google/AFL)
 * See "llvm_mode/afl-llvm-pass.so.cc" for the original implementation.
 */

#define USE_COLOR     // Yes, please.

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "stuff.h"

using namespace llvm;

class LSCovPass : public PassInfoMixin<LSCovPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};

PreservedAnalyses LSCovPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVMContext &C = M.getContext();

  IntegerType *Int8Ty  = IntegerType::getInt8Ty(C);
  PointerType *Int8PtrTy = PointerType::get(Int8Ty, 0);
  IntegerType *Int32Ty = IntegerType::getInt32Ty(C);

  /* Show a banner */
  //SAYF(cCYA "lscov-llvm-pass " cBRI VERSION cRST " by <iss300@gmail.com>\n");

  /* Get globals for the SHM region and the previous location */
  GlobalVariable *LSCovMapPtr = new GlobalVariable(
      M, Int8PtrTy, false, GlobalValue::ExternalLinkage, 0, "__lscov_area_ptr");

  GlobalVariable *LSCovPrevLoc = new GlobalVariable(
      M, Int32Ty, false, GlobalValue::ExternalLinkage, 0, "__lscov_prev_loc",
      0, GlobalVariable::GeneralDynamicTLSModel, 0, false);

  /* Instrument all the things! */
  int inst_blocks = 0;

  for (auto &F : M) {
    /* Dirty hack: if this function has anything to do with 'sancov', ignore. */
    if (F.getName().contains("sancov"))
      continue;

    for (auto &BB : F) {
      /* Skip this basic block if it terminates with an unconditional branch. */
      Instruction *TermI= BB.getTerminator();
      BranchInst *TermBrI= TermI ? dyn_cast<BranchInst>(TermI) : nullptr;
      
      if (TermBrI&& TermBrI->isUnconditional())
        continue;

      BasicBlock::iterator IP = BB.getFirstInsertionPt();
      IRBuilder<> IRB(&(*IP));

      /* Make up cur_loc */
      unsigned int cur_loc = RANDOM(LSTATE_SIZE);
      ConstantInt *CurLoc = ConstantInt::get(Int32Ty, cur_loc);

      /* Load prev_loc */
      LoadInst *PrevLoc = IRB.CreateLoad(Int32Ty, LSCovPrevLoc);
      PrevLoc->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *PrevLocCasted = IRB.CreateZExt(PrevLoc, IRB.getInt32Ty());

      /* Load SHM pointer */
      LoadInst *MapPtr = IRB.CreateLoad(Int8PtrTy, LSCovMapPtr);
      MapPtr->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *MapPtrIdx = IRB.CreateGEP(Int8Ty, MapPtr, 
          IRB.CreateXor(PrevLocCasted, CurLoc));

      /* Update bitmap */
      LoadInst *Counter = IRB.CreateLoad(Int8Ty, MapPtrIdx);
      Counter->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));
      Value *Incr = IRB.CreateAdd(Counter, ConstantInt::get(Int8Ty, 1));
      IRB.CreateStore(Incr, MapPtrIdx)
          ->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

      /* Set prev_loc to cur_loc >> 1 */
      StoreInst *Store =
          IRB.CreateStore(ConstantInt::get(Int32Ty, cur_loc >> 1), LSCovPrevLoc);
      Store->setMetadata(M.getMDKindID("nosanitize"), MDNode::get(C, None));

      inst_blocks++;
    }
  }

  /* Say something nice */
  if (!inst_blocks) WARNF("No instrumentation targets found.");
  else OKF("Instrumented %u locations (lscov, ignoring SANCOV).", inst_blocks);

  return PreservedAnalyses::all();
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LSCovPass", "v0.0",
    [](PassBuilder &PB) {
      PB.registerOptimizerLastEPCallback(
        [](ModulePassManager &MPM, OptimizationLevel OL) {
          MPM.addPass(LSCovPass());
        });
    }};
}
