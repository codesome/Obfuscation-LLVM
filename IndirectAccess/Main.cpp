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
#include "IndirectAccess.h"
using namespace llvm;

#define DEBUG_TYPE "indirect-access"

namespace {

// Runs split on inner most loop
void checkInnerMost(Loop *L, LoopInfo *LI, DominatorTree *DT, ScalarEvolution *SE, LLVMContext *CTX, Function *F) {
    // Stroing all the original loop pointes
    // Because after splitting the iteration gets affected due to new loops
    std::vector<Loop*> nestedLoops;
    for (Loop *NL : *L) {
        nestedLoops.push_back(NL);
    }
    // Recursing over the original nested loops
    for (Loop *NL : nestedLoops) {
        checkInnerMost(NL, LI, DT, SE, CTX, F);
    }
    // Splitting only if loop doesn't have nested loop
    if(nestedLoops.size() <= 0) {
        Value* loopIterator;
        if(IndirectAccessUtils::isLegalTransform(L, loopIterator)) {
            int tripCount = SE->getSmallConstantTripCount(L);
            LoopSplitInfo LSI = IndirectAccessUtils::splitAndCreateArray(L, tripCount, loopIterator, CTX, LI, DT, F);
            IndirectAccessUtils::updateIndirectAccess(&LSI);
        }
    }
}

} /* namespace */

bool IndirectAccess::runOnFunction(Function &F) {
    bool modified = true;

    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(createPromoteMemoryToRegisterPass());
    // If we enable loop rotate here, ScalarEvolution is giving error
    // Hence have -loop-rotate in command line before -indirect-access for now
    // FPM.add(createLoopRotatePass());
    // FPM.add(createLoopSimplifyPass());
    FPM.run(F);

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    LLVMContext &CTX = F.getContext();

    // Iterating only on original loops and not the ones created
    std::vector<Loop*> loops;
    for (Loop *L : LI) {
        loops.push_back(L);
    }

    // Splitting the original loops in function
    for (Loop *L : loops) {
        checkInnerMost(L, &LI, &DT, &SE, &CTX, &F);
    }

    return modified;
}

/* Only for testing (by Ganesh).
Dont uncomment or delete it.
It will be removed once its purpose is over */
/*
bool IndirectAccess::runOnFunction(Function &F) {
    bool modified = true;
    LoopSplitInfo LSI(nullptr,nullptr,nullptr);

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (Loop *L : LI) {
        LSI.clonedLoop = L;
        break;
    }

    BasicBlock &BB = F.getEntryBlock();
    int cnt = 0;
    for(Instruction &I: BB) {
        cnt++;
        if(cnt==4) {
            LSI.arrayValue = &I;
        }
        if(cnt==5) {
            LSI.tripCountValue = &I;
        }
    }

    IndirectAccessUtils::updateIndirectAccess(&LSI);

    return modified;
}
*/
void IndirectAccess::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
}


// Registering the pass
char IndirectAccess::ID = 0;
static RegisterPass<IndirectAccess> X("indirect-access", "Indirect access of loop iterators");

#undef DEBUG_TYPE
