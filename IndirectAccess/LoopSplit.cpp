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

void swapLoops(Loop *L1, Loop *L2) {

    BasicBlock *L1PreHeader = L1->getLoopPreheader();
    BasicBlock *L1PrePreHeader = L1PreHeader->getUniquePredecessor();
    BasicBlock *L2PreHeader = L2->getLoopPreheader();
    BasicBlock *L1Latch = L1->getLoopLatch();
    BasicBlock *L2Latch = L2->getLoopLatch();

    BasicBlock *end = L2Latch->getTerminator()->getSuccessor(1);
    L2Latch->getTerminator()->setSuccessor(1, L1PreHeader);
    L1Latch->getTerminator()->setSuccessor(1, end);
    L1PrePreHeader->getTerminator()->setSuccessor(0, L2PreHeader);

}

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
    // Loop* newLoop = cloneLoopWithPreheader(Before, LoopDomBB, L, VMap, Twine(".cl"), LI, DT, blocks);
    Loop* newLoop = cloneLoopWithPreheader(Before, LoopDomBB, L, VMap, Twine(".cl"), LI, DT, blocks);
    // Remapping the blocks in new loop
    remapInstructionsInBlocks(blocks, VMap);
    
    // Fixing the successor of the old block to
    // map to the preHeader of new loop
    // else it will be mapping to the new loop body
    L->getLoopLatch()->getTerminator()->setSuccessor(1,newLoop->getLoopPreheader());

    swapLoops(L, newLoop);

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
// BasicBlock* cloneBasicBlock(BasicBlock *BB, Function *F) {
//     ValueToValueMapTy VMap;
//     BasicBlock* newBasicBlock = CloneBasicBlock(BB, VMap, Twine(".cl"), F);
//     return newBasicBlock;
// }


void fixPhiNodesInBody(Loop *correct, Loop *toBeFixed) {

    BasicBlock *correctHeader = correct->getLoopPreheader();
    BasicBlock *toBeFixedHeader = toBeFixed->getLoopPreheader();
    BasicBlock *correctBody = correctHeader->getUniqueSuccessor();
    BasicBlock *toBeFixedBody = toBeFixedHeader->getUniqueSuccessor();

    std::vector<Value*> correctValues;
    for(PHINode &phi: correctBody->phis()) {
        correctValues.push_back(phi.getIncomingValueForBlock(correctHeader));
    }

    int i = 0, index;
    for(PHINode &phi: toBeFixedBody->phis()) {
        index = phi.getBasicBlockIndex(toBeFixedHeader);
        phi.setIncomingValue(index, correctValues[i]);
        i++;
    }

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

void IndirectAccessUtils::clone(LoopSplitInfo *LSI, 
    LoopInfo *LI, DominatorTree *DT) {

    // This cloned loop is used to populate the array
    LSI->clonedLoop = cloneLoop(LSI->originalLoop, LI, DT);

    BasicBlock *preHeader = LSI->clonedLoop->getLoopPreheader();
    BasicBlock *loopLatch = LSI->clonedLoop->getLoopLatch();

    LSI->originalLoopPreheader = preHeader;
    LSI->originalLoopBody = preHeader->getUniqueSuccessor();

    LSI->originalLoopLatch = loopLatch;
}

void IndirectAccessUtils::clearClonedLoop(LoopSplitInfo *LSI) {

    BasicBlock *preHeader = LSI->clonedLoop->getLoopPreheader();
    BasicBlock *loopLatch = LSI->clonedLoop->getLoopLatch();
    bool firstBody = true;
    for(BasicBlock *BB: LSI->clonedLoop->getBlocks()) {
        if(BB!=preHeader && BB!=loopLatch) {
            for(Instruction &I : *BB) {
                if(!dyn_cast<PHINode>(&I) || !firstBody) {
                    I.replaceAllUsesWith(UndefValue::get(I.getType()));
                }
            }
            firstBody = false;
        }
    }

    fixPhiNodesInBody(LSI->clonedLoop, LSI->originalLoop);

}

Value* IndirectAccessUtils::getIterator(Loop *L) {
    BasicBlock *preHeader = L->getLoopPreheader();
    Value *iterator;
    for(PHINode &phi: preHeader->getUniqueSuccessor()->phis()) {
        iterator = dyn_cast<Value>(&phi);
    }
    return iterator;
}