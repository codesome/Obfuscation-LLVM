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

    // The original loop
    Loop* originalLoop;

    // The cleared basic block in original loop where the indirect access array will be populated
    BasicBlock* originalLoopBody;

    // Preheader of original loop
    BasicBlock* originalLoopPreheader;
    // Latch of original loop
    BasicBlock* originalLoopLatch;

    // The cloned loop where the indirect access will be done
    Loop* clonedLoop;

    // The new array created for indirect access
    Value* arrayValue;

    // While updating array we keep a count in IR
    // this is that value which is used for iterating 
    // for indirect access
    Value* tripCountValue;

    // iterator of the loop
    Value* iterator;

    unsigned int tripCount;

    LoopSplitInfo(Loop* originalLoop):
        originalLoop(originalLoop), 
        clonedLoop(nullptr), 
        arrayValue(nullptr), 
        tripCountValue(nullptr) {}
    
    LoopSplitInfo(Loop* originalLoop, Loop* clonedLoop, Value* arrayValue, Value* tripCountValue):
        originalLoop(originalLoop),
        clonedLoop(clonedLoop), 
        arrayValue(arrayValue), 
        tripCountValue(tripCountValue) {}
};

class IndirectAccessUtils {
public:
    /*_______________________________________________________________________
     *
     * Used to check for legality of indirect access of iterator
     *
     * @param Loop* L, the loop to check for legality
     * @param Value* loopIterator, this should be empty. 
     *      This will be populated with the loop iterator if result is legal
     *
     * @return true if its legal, else false
     *______________________________________________________________________*/
    static bool isLegalTransform(Loop *L, Value* loopIterator);

    /*______________________________________________________________________
     *
     * Clones the original loop and clears the body of the original loop
     * The original loop will enter cloned loop after it exits
     * NOTE: The analysis pass will be invalid after this function call
     *
     * @param LoopSplitInfo *LSI, which constains orginalLoop
     * @param LoopInfo *LI, Loop info from analysis pass
     * @param DominatorTree *DT, from analysis pass
     * @param Function *F, functon in which the loop is present
     *______________________________________________________________________*/
    static void cloneAndClearOriginal(LoopSplitInfo *LSI, 
        LoopInfo *LI, DominatorTree *DT, Function *F);

    /*______________________________________________________________________
     *
     * Allocate array and count and update the array
     * inside the original loop body
     * 
     * @param LoopSplitInfo *LSI, which constains orginalLoop
     * @param LoopInfo *LI, Loop info from analysis pass
     * @param DominatorTree *DT, from analysis pass
     * @param Function *F, functon in which the loop is present
     *______________________________________________________________________*/
    static void initialiseAndUpdateArray(LoopSplitInfo *LSI, 
        LoopInfo *LI, DominatorTree *DT, Function *F);

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
