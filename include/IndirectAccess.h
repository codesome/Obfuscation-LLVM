#ifndef __INDIRECT_ACCESS_H__
#define __INDIRECT_ACCESS_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"
using namespace llvm;


struct LoopSplitInfo {

    // The cloned loop where the indirect access will be done
    Loop* clonedLoop;

    // The new array created for indirect access
    Value* arrayValue;

    // While updating array we keep a count in IR
    // this is that value which is used for iterating 
    // for indirect access
    Value* tripCountValue;

    LoopSplitInfo(): clonedLoop(nullptr), arrayValue(nullptr), tripCountValue(nullptr) {}

    LoopSplitInfo(Loop* clonedLoop, Value* arrayValue, Value* tripCountValue):
        clonedLoop(clonedLoop), 
        arrayValue(arrayValue), 
        tripCountValue(tripCountValue) {}
};

class IndirectAccessUtils {
public:
    /*
     * Used to check for legality of indirect access of iterator
     *
     * @param Loop* L, the loop to check for legality
     * @param Value* loopIterator, this should be empty. 
     *      This will be populated with the loop iterator if result is legal
     *
     * @return true if its legal, else false
     **/
    static bool isLegalTransform(Loop *L, Value* loopIterator);

    static LoopSplitInfo splitAndCreateArray(Loop *L, int tripCount, 
        Value *loopIterator, LoopInfo *LI, DominatorTree *DT, Function *F);

    static void updateIndirectAccess(LoopSplitInfo* LSI);

};

class IndirectAccess : public FunctionPass {

public:
    static char ID;

    IndirectAccess() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);
    
    void getAnalysisUsage(AnalysisUsage &AU) const override;

};

#endif
