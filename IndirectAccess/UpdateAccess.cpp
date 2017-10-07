#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

void IndirectAccessUtils::updateIndirectAccess(LoopSplitInfo* LSI, Function* F) {
	if(LSI->clonedLoop==nullptr)
		return;
	
    // L is the cloned loop where indirect access is to be done.
    Loop* L = LSI->clonedLoop;
    Value* tripcount = LSI->tripCountValue;  // Loop trip count
    BasicBlock *preHeader = L->getLoopPreheader(); // Loop preheader
    BasicBlock *loopLatch = L->getLoopLatch();  // Loop latch


    // Creating a new variable to iterate loop initialized with 0
    Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(preHeader->getTerminator());
    Value* it = headerBuilder.CreateAlloca(i32, zero);
    it->setName("it");

    // Adding instructions in loop latch
    IRBuilder<> latchBuilder(loopLatch->getFirstNonPHI());
    Value *itLoad = latchBuilder.CreateLoad(it);
    Value* one = ConstantInt::get(i32, 1);
    Value *increment = latchBuilder.CreateAdd(itLoad, one);
    latchBuilder.CreateStore(increment, it);
    Value *itnew = latchBuilder.CreateLoad(it);
    Value *cmpInst = latchBuilder.CreateICmpSLT(itnew,tripcount);

    // Replacing old iterator with new one
    Instruction* I = L->getLoopLatch()->getTerminator();
    I->setOperand(0,cmpInst);
    
    // Replacing the old variable with new one
    Value* zero = ConstantInt::get(i32, 0);
    for(BasicBlock *B : L->getBlocks()) {
        if(B!=preHeader && B!=loopLatch) {
            // blocks which are not header and not latch
            IRBuilder<> blockBuilder(B->getFirstNonPHI());

            // Getting the value from the array from the new index
            Value *itLoad = blockBuilder.CreateLoad(it);
            std::vector<Value*> idxVector;
            idxVector.push_back(zero);
            idxVector.push_back(itLoad);
            ArrayRef<Value*> idxList(idxVector);
            Value *arrayIdx = blockBuilder.CreateGEP(LSI->arrayValue, idxList);

            // Replacing the value of 'i' with 'array[it]'
            Value *i_new = blockBuilder.CreateLoad(arrayIdx);
            BasicBlock::iterator ii(LSI->iterator);
            ReplaceInstWithValue(B->getInstList(), ii,i_new);

    	}
    }
}
