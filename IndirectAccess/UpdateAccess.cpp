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


    IRBuilder<> Builder(preHeader->getUniqueSuccessor()->getFirstNonPHI());


    PHINode *phi = Builder.CreatePHI(i32, 2, Twine(".new"));
    phi->addIncoming(zero, preHeader);
    
    
    IRBuilder<> latchBuilder(loopLatch->getTerminator());
    
    /*Adding phi intruction to for compare operator*/
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

    //Replacing all the uses of previous iterator with new one
    iterator->replaceAllUsesWith(indirectAccess);

    // Changing compare operand
    Value *tripcnt = latchBuilder.CreateLoad(tripcount);
    Value *cmpInst = latchBuilder.CreateICmpSLT(increment,tripcnt);
    Instruction* I = L->getLoopLatch()->getTerminator();
    I->setOperand(0,cmpInst);


}
