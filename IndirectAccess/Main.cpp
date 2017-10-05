#include "llvm/Pass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "IndirectAccess.h"
using namespace llvm;

#define DEBUG_TYPE "indirect-access"

namespace {

// Runs split on inner most loop
void splitInnerMost(Loop *L, LoopInfo *LI, DominatorTree *DT) {
    // Stroing all the original loop pointes
    // Because after splitting the iteration gets affected due to new loops
    std::vector<Loop*> nestedLoops;
    for (Loop *NL : *L) {
        nestedLoops.push_back(NL);
    }
    // Splitting the original loops
    for (Loop *NL : nestedLoops) {
        splitInnerMost(NL, LI, DT);
    }
    // Splitting only if it doesnt have nested loop
    if(nestedLoops.size() <= 0) {
        int tripCount = 1;
        if(CheckLegality::isLegalTransform(L)) {
            LoopSplitInfo LSI = LoopSplit::splitAndCreateArray(L, tripCount, LI, DT);
            UpdateAccess::updateIndirectAccess(&LSI);
        }
    }
}

} /* namespace */

bool IndirectAccess::runOnFunction(Function &F) {
    bool modified = true;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    // Iterating only on original loops and not the ones created
    std::vector<Loop*> loops;
    for (Loop *L : LI) {
        loops.push_back(L);
    }

    // Splitting the original loops in function
    for (Loop *L : loops) {
        splitInnerMost(L, &LI, &DT);
    }

    return modified;
}

void IndirectAccess::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
}


// Registering the pass
char IndirectAccess::ID = 0;
static RegisterPass<IndirectAccess> X("indirect-access", "Indirect access of loop iterators");

#undef DEBUG_TYPE
