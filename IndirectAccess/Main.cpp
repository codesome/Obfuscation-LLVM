#include "llvm/Pass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "IndirectAccess.h"
using namespace llvm;

#define DEBUG_TYPE "indirect-access"

namespace {

// This is a temporary function
Value* getIterator(Loop* L) {
    Instruction *I = dyn_cast<Instruction>(L->getLoopLatch()->getTerminator()->getOperand(0));
    return dyn_cast<AllocaInst>(dyn_cast<Instruction>(I->getOperand(0))->getOperand(0));
}

// Runs split on inner most loop
void checkInnerMost(Loop *L, LoopInfo *LI, DominatorTree *DT, 
    Function *F, std::vector<LoopSplitInfo*> *lsi) {
    // Stroing all the original loop pointes
    // Because after splitting the iteration gets affected due to new loops
    std::vector<Loop*> nestedLoops;
    for (Loop *NL : *L) {
        nestedLoops.push_back(NL);
    }
    // Recursing over the original nested loops
    for (Loop *NL : nestedLoops) {
        checkInnerMost(NL, LI, DT, F, lsi);
    }
    // Splitting only if loop doesn't have nested loop
    if(nestedLoops.size() <= 0) {
        Value* loopIterator;
        if(IndirectAccessUtils::isLegalTransform(L, loopIterator)) {
            LoopSplitInfo *LSI = new LoopSplitInfo(L);
            LSI->iterator = getIterator(L);
            lsi->push_back(LSI);
            IndirectAccessUtils::clone(LSI, LI, DT);
        }
    }
}

} /* namespace */

bool IndirectAccess::runOnFunction(Function &F) {
    bool modified = true;

    legacy::FunctionPassManager FPM(F.getParent());
    // If we enable loop rotate here, ScalarEvolution is giving error
    // Hence have -loop-rotate in command line before -indirect-access for now
    // FPM.add(createLoopRotatePass());
    FPM.add(createLoopSimplifyPass());
    FPM.run(F);

    // Iterating only on original loops and not the ones created
    std::vector<LoopSplitInfo*> lsi;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    // Splitting the original loops in function
    // FPM.add(createUnreachableBlockEliminationPass());
    for (Loop *L : LI) {
        checkInnerMost(L, &LI, &DT, &F, &lsi);
    }

    FPM.add(createPromoteMemoryToRegisterPass());
    FPM.run(F);
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    int maxTripCount = 0;
    int tripCount;
    for(LoopSplitInfo *LSI : lsi) {
        tripCount = SE.getSmallConstantTripCount(LSI->clonedLoop);
        if(tripCount > maxTripCount) {
            maxTripCount = tripCount;
        }
    }

    Value *array = IndirectAccessUtils::allocateArrayInEntryBlock(&F, maxTripCount);

    // allocate array in global
    for(LoopSplitInfo *LSI : lsi) {
        dbgs() << SE.getSmallConstantTripCount(LSI->clonedLoop) << "\n";
        LSI->tripCount = SE.getSmallConstantTripCount(LSI->clonedLoop);
        IndirectAccessUtils::clearClonedLoop(LSI);
        IndirectAccessUtils::initialiseAndUpdateArray(LSI, &LI, &DT, &F, array);
        // IndirectAccessUtils::updateIndirectAccess(LSI, &F);
    }
    FPM.add(createDeadInstEliminationPass());
    FPM.run(F);

    return modified;
}

void IndirectAccess::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
}


// Registering the pass
char IndirectAccess::ID = 0;
static RegisterPass<IndirectAccess> X("indirect-access", "Indirect access of loop iterators");

#undef DEBUG_TYPE
