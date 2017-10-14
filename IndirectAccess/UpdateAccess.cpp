#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "IndirectAccess.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

void IndirectAccessUtils::updateIndirectAccess(LoopSplitInfo* LSI, Function* F, Value *array, ScalarEvolution *SE) {
	if(LSI->clonedLoop==nullptr)
		return;
    
    Value *iterator = getIterator(LSI->originalLoop, SE);
    // L is the cloned loop where indirect access is to be done.
    Loop* L = LSI->originalLoop;
    Value* tripcount = LSI->tripCountValue;  // Loop trip count
    BasicBlock *preHeader = L->getLoopPreheader(); // Loop preheader
    BasicBlock *loopLatch = L->getLoopLatch();  // Loop latch


    // Creating a new variable to iterate loop initialized with 0
    Type* i32 = Type::getInt32Ty(F->getContext());
    Value* zero = ConstantInt::get(i32, 0);
    IRBuilder<> headerBuilder(preHeader->getUniqueSuccessor()->getFirstNonPHI());
    // Value* it = headerBuilder.CreateAlloca(i32, zero);
    // it->setName("it");

    // Adding instructions in loop latch
    
    
    // Value *increment = latchBuilder.CreateAdd(itLoad, one);
    // latchBuilder.CreateStore(increment, it);
    // Value *itnew = latchBuilder.CreateLoad(it);
    // Value *tripcnt = latchBuilder.CreateLoad(tripcount);
    // Value *cmpInst = latchBuilder.CreateICmpSLT(itnew,tripcnt);
    
    PHINode *phi1 = headerBuilder.CreatePHI(i32, 2, Twine(".new"));

    dbgs()<< *phi1<<"\n";
    phi1->addIncoming(zero, preHeader);
    
    dbgs()<< *phi1<<"\n";

    IRBuilder<> latchBuilder(loopLatch->getTerminator());
    // Value *itLoad = latchBuilder.CreateLoad(it);
    Value* one = ConstantInt::get(i32, 1);
    Value *increment = latchBuilder.CreateAdd(phi1, one);
    phi1->addIncoming(increment,loopLatch);
    // Replacing old iterator with new one
    // Instruction* I = L->getLoopLatch()->getTerminator();
    // I->setOperand(0,cmpInst);
    
    // Replacing the old variable with new one
    /*
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
            Value *arrayIdx = blockBuilder.CreateGEP(array, idxList);

            // Replacing the value of 'i' with 'array[it]'
            Value *i_new = blockBuilder.CreateLoad(arrayIdx);
            blockBuilder.CreateStore(i_new,iterator);

    	}
    }
    */
}
