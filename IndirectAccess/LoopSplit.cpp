#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/ADT/Twine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/APInt.h"
#include "IndirectAccess.h"
using namespace llvm;

namespace {

/*_______________________________________________________________
 *
 * After cloning, cloned loop is put after the original
 * This function moves the cloned loop before the original loop
 *
 * @param Loop *L1, the original loop (the first loop)
 * @param Loop *L2, the cloned loop (the second loop)
 *_______________________________________________________________*/
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

/*___________________________________________________________________________________
 *
 * After changing all uses to undef, phi nodes are affected in the loop following it.
 * Updating the value with values from the cloned loop
 *
 * @param Loop *correct, the cloned loop where loop was cleared
 * @param Loop *toBeFixed, the original loop where phi nodes got undef values
 *_________________________________________________________________________________*/
void fixPhiNodesInBody(Loop *correct, Loop *toBeFixed) {

    BasicBlock *correctHeader = correct->getLoopPreheader();
    BasicBlock *toBeFixedHeader = toBeFixed->getLoopPreheader();
    BasicBlock *correctBody = correctHeader->getUniqueSuccessor();
    BasicBlock *toBeFixedBody = toBeFixedHeader->getUniqueSuccessor();

    // collecting correct values from cloned loop for its preheader
    std::vector<Value*> correctValues;
    for(PHINode &phi: correctBody->phis()) {
        correctValues.push_back(phi.getIncomingValueForBlock(correctHeader));
    }

    // The cloned and original loops have phi nodes in same order
    // Updating phi node values for pre header of original loop with pre header for clone loop
    int i = 0, index;
    for(PHINode &phi: toBeFixedBody->phis()) {
        index = phi.getBasicBlockIndex(toBeFixedHeader);
        phi.setIncomingValue(index, correctValues[i]);
        i++;
    }

}

} /* namespace */

void IndirectAccessUtils::populateArray(LoopSplitInfo *LSI, 
    Function *F,Value *indirectAccessArray, ScalarEvolution *SE) {
    
    Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(LSI->clonedLoop->getLoopPreheader()->getTerminator());

    // Initialising cnt = 0 in loop pre header
    // This is the runtime trip count of the loop to avoid any runtime errors
    // This count is used as loop bound for during indirect access
    AllocaInst* cnt = headerBuilder.CreateAlloca(i32);
    headerBuilder.CreateStore(zero, cnt);
    // cnt->setAlignment(4);

    // array[cnt] = iter,  iterator in loop body
    IRBuilder<> bodyBuilder(LSI->clonedLoop->getLoopPreheader()
        ->getUniqueSuccessor()->getTerminator());
    // iter
    Value *iterLoad = getIntegerIterator(LSI->clonedLoop, SE);
    
    // cnt
    Value *countLoadBody = bodyBuilder.CreateLoad(cnt);
    
    // array[cnt]
    // GEP needs '0, cnt'
    std::vector<Value*> idxVector;
    idxVector.push_back(zero); // 0
    idxVector.push_back(countLoadBody); // cnt
    ArrayRef<Value*> idxList(idxVector);
    Value *arrayIdx = bodyBuilder.CreateGEP(indirectAccessArray, idxList);

    // array[cnt] = iter
    unsigned int iBits = iterLoad->getType()->getPrimitiveSizeInBits();
    if(iBits != IndirectAccessUtils::MAX_BITS) {
        Type* iMAX = Type::getIntNTy(F->getContext(), IndirectAccessUtils::MAX_BITS);
        iterLoad = bodyBuilder.CreateZExt(iterLoad, iMAX);
    }
    bodyBuilder.CreateStore(iterLoad, arrayIdx);

    // Type* m = Type::getIntNTy(F->getContext(), iBits);
    // auto *Trunc = bodyBuilder.CreateTrunc(iterLoad, m);

    // cnt++ in loop latch
    Value* one = ConstantInt::get(i32, 1);
    IRBuilder<> latchBuilder(LSI->clonedLoop->getLoopLatch()->getTerminator());
    Value *countLoadLatch = latchBuilder.CreateLoad(cnt);
    Value *increment = latchBuilder.CreateAdd(countLoadLatch, one);
    latchBuilder.CreateStore(increment, cnt);

    // runtime trip count
    LSI->tripCountValue = cnt;

}

void IndirectAccessUtils::clone(LoopSplitInfo *LSI, 
    LoopInfo *LI, DominatorTree *DT) {
    // This cloned loop is used to populate the array
    LSI->clonedLoop = cloneLoop(LSI->originalLoop, LI, DT);
}

void IndirectAccessUtils::clearClonedLoop(LoopSplitInfo *LSI) {

    BasicBlock *preHeader = LSI->clonedLoop->getLoopPreheader();
    BasicBlock *loopLatch = LSI->clonedLoop->getLoopLatch();
    
    // Making all uses of instructions (except preheader and loop latch) to undef
    // NOTE: Uses of phi nodes in the first BasicBlock of body are not made undef
    //       as we need it for updating indirect access
    // All instructions of rest of the BasicBlock in the body is made undef
    bool firstBody = true;
    for(BasicBlock *BB: LSI->clonedLoop->getBlocks()) {
        if(BB!=preHeader && BB!=loopLatch) {
            for(BasicBlock::iterator DI = BB->begin(); DI != BB->end();) {
                Instruction *I = &*DI++;
                if( BB->getTerminator()!=I && (!dyn_cast<PHINode>(I) || !firstBody)) {
                    I->replaceAllUsesWith(UndefValue::get(I->getType()));
                    I->eraseFromParent();
                }
            }
            firstBody = false;
        }
    }

    fixPhiNodesInBody(LSI->clonedLoop, LSI->originalLoop);

}

Value* IndirectAccessUtils::getIntegerIterator(Loop *L, ScalarEvolution *SE) {
    // Getting the first induction phi
    BasicBlock *preHeader = L->getLoopPreheader();
    for(PHINode &phi: preHeader->getUniqueSuccessor()->phis()) {
        InductionDescriptor ID;
        if(InductionDescriptor::isInductionPHI(&phi, L, SE, ID)) {
            Type *ty = phi.getType();
            if(ty->isIntegerTy() && ty->getPrimitiveSizeInBits() <= IndirectAccessUtils::MAX_BITS)
                return dyn_cast<Value>(&phi);
        }
    }
    return nullptr;
}

Value* IndirectAccessUtils::allocateArrayInEntryBlock(Function *F, int size) {
    BasicBlock &entryBlock = F->getEntryBlock();
    IRBuilder<> builder(entryBlock.getTerminator());

    // Initialising array in loop pre header for indirect access
    Type* iMAX = Type::getIntNTy(F->getContext(), IndirectAccessUtils::MAX_BITS);
    AllocaInst* indirectAccessArray = builder.CreateAlloca(ArrayType::get(iMAX, size));
    // indirectAccessArray->setAlignment(16);
    return indirectAccessArray;
}
