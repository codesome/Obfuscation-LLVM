#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "IndirectAccess/IndirectAccess.h"
using namespace llvm;

void IndirectAccessUtils::updateIndirectAccess(LoopSplitInfo* LSI, Function* F, Value *array, ScalarEvolution *SE) {
	if(LSI->clonedLoop==nullptr)
		return;
    
    Value *iterator = getIntegerIterator(LSI->originalLoop, SE);
    // L is the cloned loop where indirect access is to be done.
    Loop* L = LSI->originalLoop;
    Value* tripcount = LSI->tripCountValue;  // Loop trip count
    BasicBlock *preHeader = L->getLoopPreheader(); // Loop preheader
    BasicBlock *loopLatch = L->getLoopLatch();  // Loop latch

    // Creating a new variable to iterate loop, initialized with 0
    Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);

    // Adding out phi node for iteration in the begining of the loop body
    IRBuilder<> Builder(preHeader->getUniqueSuccessor()->getFirstNonPHI());
    PHINode *phi = Builder.CreatePHI(i32, 2);
    phi->addIncoming(zero, preHeader);

    // For incrementing the iterator and changing conditions of loop
    IRBuilder<> latchBuilder(loopLatch->getTerminator());
    Value* one = ConstantInt::get(i32, 1);
    Value *increment = latchBuilder.CreateAdd(phi, one);
    phi->addIncoming(increment,loopLatch);

    // Adding getelementptr to get the value from the array
    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(phi);
    ArrayRef<Value*> idxList(idxVector);
    Value *arrayIdx = Builder.CreateGEP(array, idxList);
    Value *indirectAccess = Builder.CreateLoad(arrayIdx);
    
    // Fixing the bits in the integer
    unsigned int iBits = iterator->getType()->getPrimitiveSizeInBits();
    if(iBits != IndirectAccessUtils::MAX_BITS) {
        // integer was smaller, hence truncate
        Type* iOriginal = Type::getIntNTy(F->getContext(), iBits);
        indirectAccess = Builder.CreateTrunc(indirectAccess, iOriginal);
    }

    //Replacing all the uses of previous iterator with new one
    iterator->replaceAllUsesWith(indirectAccess);

    // Changing compare imstruction of the loop
    Value *tripcnt = latchBuilder.CreateLoad(tripcount);
    Value *cmpInst = latchBuilder.CreateICmpSLT(increment,tripcnt);
    Instruction* I = L->getLoopLatch()->getTerminator();
    I->setOperand(0,cmpInst);

}
