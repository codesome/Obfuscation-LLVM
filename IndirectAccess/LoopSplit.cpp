#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/APInt.h"
#include "IndirectAccess.h"
using namespace llvm;

namespace {

/*___________________________________________________________
 *
 * Clones a loop
 *
 * @param Loop *L, the loop to be cloned
 * @param LoopInfo *LI, got from analysis pass
 * @param DominatorTree *DT, got from analysis pass
 *
 * @return Loop*, the cloned loop
 *___________________________________________________________*/
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

/*___________________________________________________________
 *
 * Clones a basic block
 *
 * @param BasicBlock *BB, the basic block to be cloned
 * @param Function *F, the function in which BB exists
 *
 * @return BasicBlock*, the cloned basic block
 *__________________________________________________________*/
BasicBlock* cloneBasicBlock(BasicBlock *BB, Function *F) {
    ValueToValueMapTy VMap;
    BasicBlock* newBasicBlock = CloneBasicBlock(BB, VMap, Twine(".cl"), F);
    return newBasicBlock;
}

} /* namespace */

void IndirectAccessUtils::initialiseAndUpdateArray(LoopSplitInfo *LSI, 
    LoopInfo *LI, DominatorTree *DT, Function *F) {

    Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(LSI->originalLoopPreheader->getTerminator());

    // Initialising cnt = 0 in loop pre header
    // This is the runtime trip count of the loop to avoid any runtime errors
    // This count is used as loop bound for during indirect access
    Value* cnt = headerBuilder.CreateAlloca(i32, zero);
    cnt->setName("cnt");

    // Initialising array in loop pre header for indirect access
    Value* indirectAccessArray = headerBuilder.CreateAlloca(ArrayType::get(i32, LSI->tripCount));
    indirectAccessArray->setName("ia");

    // array[cnt] = iter,  iterator in loop body
    IRBuilder<> bodyBuilder(LSI->originalLoopBody->getTerminator());
    // iter
    Value *iterLoad = bodyBuilder.CreateLoad(LSI->iterator);
    // cnt
    Value *countLoad1 = bodyBuilder.CreateLoad(cnt);
    
    // array[cnt]
    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(countLoad1);
    ArrayRef<Value*> idxList(idxVector);
    Value *arrayIdx = bodyBuilder.CreateGEP(indirectAccessArray, idxList);

    // array[cnt] = iter
    bodyBuilder.CreateStore(iterLoad, arrayIdx);

    // cnt++ in loop latch
    Value* one = ConstantInt::get(i32, 1);
    IRBuilder<> latchBuilder(LSI->originalLoopLatch->getTerminator());
    Value *countLoad2 = latchBuilder.CreateLoad(cnt);
    Value *increment = latchBuilder.CreateAdd(countLoad2, one);
    latchBuilder.CreateStore(increment, cnt);

    LSI->tripCountValue = cnt;
    LSI->arrayValue = indirectAccessArray;

}

void IndirectAccessUtils::cloneAndClearOriginal(LoopSplitInfo *LSI, 
    LoopInfo *LI, DominatorTree *DT, Function *F) {

    // This cloned loop is used to populate the array
    LSI->clonedLoop = cloneLoop(LSI->originalLoop, LI, DT);

    BasicBlock *preHeader = LSI->originalLoop->getLoopPreheader();
    BasicBlock *loopLatch = LSI->originalLoop->getLoopLatch();

    // Inserting the cloned block in the original loop
    // After this the original blocks inside loop 
    // will become unreachable. Need to remove them manualy.
    BasicBlock* clonedBB = cloneBasicBlock(preHeader, F);
    preHeader->getTerminator()->setSuccessor(0, clonedBB);
    loopLatch->getTerminator()->setSuccessor(0, clonedBB);
    clonedBB->getTerminator()->setSuccessor(0, loopLatch);

    LSI->originalLoopPreheader = preHeader;
    LSI->originalLoopBody = preHeader->getUniqueSuccessor();

    LSI->originalLoopLatch = loopLatch;
}
