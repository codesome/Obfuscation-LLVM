#ifndef __INDIRECT_ACCESS_H__
#define __INDIRECT_ACCESS_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/LLVMContext.h"
using namespace llvm;


struct LoopSplitInfo {

    // The original loop where indirect will be done
    Loop* originalLoop;

    // The cloned loop where the array will be populated for indirect access
    Loop* clonedLoop;

    // While updating array we keep a runtime count in loop.
    // This is that value which is used for iterating in original loop for indirect access.
    Value* tripCountValue;

    LoopSplitInfo(Loop* originalLoop):
        originalLoop(originalLoop), 
        clonedLoop(nullptr), 
        tripCountValue(nullptr) {}
};

class IndirectAccessUtils {
public:
    static const unsigned int MAX_BITS = 64;

    /*_______________________________________________________________________
     *
     * Used to check for legality of indirect access of iterator
     *
     * @param Loop* L, the loop to check for legality
     *
     * @return true if its legal, else false
     *______________________________________________________________________*/
    static bool isLegalTransform(Loop *L, ScalarEvolution *SE);

    /*______________________________________________________________________
     *
     * Clones the original loop. After the clone, the cloned loop
     * will be entered first which exits into the original loop
     *
     * @param LoopSplitInfo *LSI, which constains orginalLoop
     * @param LoopInfo *LI, Loop info from analysis pass
     * @param DominatorTree *DT, from analysis pass
     *______________________________________________________________________*/
    static void clone(LoopSplitInfo *LSI, LoopInfo *LI, DominatorTree *DT);


    /*______________________________________________________________________
     *
     * Clears the body of the cloned loop
     * Except the induction variables
     *
     * @param LoopSplitInfo *LSI, which constains orginalLoop
     *______________________________________________________________________*/
    static void clearClonedLoop(LoopSplitInfo *LSI);

    /*______________________________________________________________________
     *
     * Allocates an array of given size in entry block
     * 
     * @param Function *F, functon in which the loop is present
     * @param int size, size of array to be allocated
     *
     * @return Value*, the allocated array
     *______________________________________________________________________*/
    static Value* allocateArrayInEntryBlock(Function *F, int size);

    /*______________________________________________________________________
     *
     * Allocate array and count and update the array
     * inside the original loop body
     * 
     * @param LoopSplitInfo *LSI, which constains orginalLoop
     * @param Function *F, functon in which the loop is present
     * @param Value *indirectAccessArray, which is the array allocated in entry block
     * @param ScalarEvolution *SE, from analysis pass
     *______________________________________________________________________*/
    static void populateArray(LoopSplitInfo *LSI, Function *F, Value *indirectAccessArray, ScalarEvolution *SE);

    /*______________________________________________________________________
     *
     * Updates indirect access in original loop
     * 
     * @param LoopSplitInfo *LSI, which constains orginal and cloned loop
     * @param Function *F, functon in which the loop is present
     *______________________________________________________________________*/
    static void updateIndirectAccess(LoopSplitInfo* LSI, Function* F, Value *indirectAccessArray, ScalarEvolution *SE);

    /*______________________________________________________________________
     *
     * Gives the first integer induction variable from the loop body
     * (integer with <= MAX_BITS)
     * 
     * @param Loop *L, loop whose induction variable is required
     * @param ScalarEvolution *SE, from analysis pass
     *
     * @return Value*, the loop iterator
     * NOTE: It returns nullptr if iterator was not found
     *______________________________________________________________________*/
    static Value* getIntegerIterator(Loop *L, ScalarEvolution *SE);

};

class IndirectAccess : public FunctionPass {

public:
    static char ID;

    IndirectAccess() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);
    
    void getAnalysisUsage(AnalysisUsage &AU) const override;

};

#endif
