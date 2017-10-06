#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
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

// TODO: test it
BasicBlock* cloneBasicBlock(BasicBlock *BB, Function *F) {
    ValueToValueMapTy VMap;
    BasicBlock* newBasicBlock = CloneBasicBlock(BB, VMap, Twine(".cl"), F);
    return newBasicBlock;
}

} /* namespace */

LoopSplitInfo IndirectAccessUtils::splitAndCreateArray(Loop *L, int tripCount, 
	Value *loopIterator, LLVMContext *CTX, LoopInfo *LI, DominatorTree *DT, Function *F) {
	
	LoopSplitInfo LSI(nullptr,nullptr,nullptr);

	LSI.clonedLoop = cloneLoop(L, LI, DT);

	BasicBlock *preHeader = L->getLoopPreheader();

	// TODO: allocate array of size trip count
    
    // Initialising cnt = 0 in loop pre header
	Type* i32 = Type::getInt32Ty(*CTX);
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(preHeader->getTerminator());
    Value* cnt = headerBuilder.CreateAlloca(i32, zero);
    cnt->setName("cnt");

	BasicBlock *loopLatch = L->getLoopLatch();
    
    // cnt++ in loop latch
    Value* one = ConstantInt::get(i32, 1);
    IRBuilder<> latchBuilder(loopLatch->getTerminator());
    Value *countLoad = latchBuilder.CreateLoad(cnt);
    Value *increment = latchBuilder.CreateAdd(countLoad, one);
    latchBuilder.CreateStore(increment, cnt);

    LSI.tripCountValue = cnt;


	// TODO: clear all blocks between header and latch
	// cloneBasicBlock(preHeader, F);


	return LSI;
}
