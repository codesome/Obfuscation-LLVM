#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "IndirectAccess.h"
using namespace llvm;

namespace {

/*
 * Clones a loop
 *
 * @param Loop *L, the loop to be cloned
 * @param LoopInfo *LI, got from analysis pass
 * @param DominatorTree *DT, got from analysis pass
 *
 * @return Loop*, the cloned loop
 */
Loop* cloneLoop(Loop *L, LoopInfo *LI, DominatorTree *DT) {

    // Before and after block for the cloned loop
    BasicBlock *Before = L->getUniqueExitBlock();
    BasicBlock *LoopDomBB = Before->getUniqueSuccessor();

    ValueToValueMapTy VMap;
    SmallVector< BasicBlock *, 8> blocks;
    
    // Cloning the loop
    Loop* newLoop = cloneLoopWithPreheader(Before, LoopDomBB, L, VMap, Twine(".cl"), LI, DT, blocks);
    // Remapping the blocks in new loop
    remapInstructionsInBlocks(blocks, VMap);
    
    // Fixing the successor of the old block to
    // map to the preHeader of new loop
    // else it will be mapping to the new loop body
    L->getLoopLatch()->getTerminator()->setSuccessor(1,newLoop->getLoopPreheader());

    return newLoop;
}

BasicBlock* cloneBasicBlock(BasicBlock *BB, Function *F) {
    ValueToValueMapTy VMap;
    BasicBlock* newBasicBlock = CloneBasicBlock(BB, VMap, Twine(".cl"), F);
    return newBasicBlock;
}

} /* namespace */

void IndirectAccessUtils::initialiseAndUpdateArray(LoopSplitInfo *LSI, 
	LoopInfo *LI, DominatorTree *DT, Function *F) {

	BasicBlock *preHeader = LSI->originalLoopPreheader;

	Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(preHeader->getTerminator());

    // Initialising cnt = 0 and array in loop pre header
    // This is the runtime trip count of the loop to avoid any runtime errors
    // This count is used as loop bound for during indirect access
    Value* cnt = headerBuilder.CreateAlloca(i32, zero);
    cnt->setName("cnt");

    // Initialising array in loop pre header for indirect access
    Value* indirectAccessArray = headerBuilder.CreateAlloca(ArrayType::get(i32, LSI->tripCount));
    indirectAccessArray->setName("ia");

	BasicBlock *loopLatch = LSI->originalLoopLatch;
    
    // cnt++ in loop latch
    Value* one = ConstantInt::get(i32, 1);
    IRBuilder<> latchBuilder(loopLatch->getTerminator());
    Value *countLoad = latchBuilder.CreateLoad(cnt);
    Value *increment = latchBuilder.CreateAdd(countLoad, one);
    latchBuilder.CreateStore(increment, cnt);

    LSI->tripCountValue = cnt;
    LSI->arrayValue = indirectAccessArray;

}

void IndirectAccessUtils::cloneAndClearOriginal(LoopSplitInfo *LSI, 
	LoopInfo *LI, DominatorTree *DT, Function *F) {

	LSI->clonedLoop = cloneLoop(LSI->originalLoop, LI, DT);

	BasicBlock *preHeader = LSI->originalLoop->getLoopPreheader();
	BasicBlock *loopLatch = LSI->originalLoop->getLoopLatch();

	BasicBlock* clonedBB = cloneBasicBlock(preHeader, F);
	preHeader->getTerminator()->setSuccessor(0, clonedBB);
	loopLatch->getTerminator()->setSuccessor(0, clonedBB);
	clonedBB->getTerminator()->setSuccessor(0, loopLatch);

	LSI->originalLoopPreheader = preHeader;
	LSI->originalLoopBody = preHeader->getUniqueSuccessor();

	LSI->originalLoopLatch = loopLatch;
}
