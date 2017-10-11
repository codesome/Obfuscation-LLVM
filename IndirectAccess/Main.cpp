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

// TODO: add proper comments
namespace {

// Runs split on inner most loop
void getInnerMostLoops(Loop *L, LoopInfo *LI, DominatorTree *DT, 
    Function *F, std::vector<LoopSplitInfo*> *lsi) {
    // Stroing all the original loop pointes
    // Because after splitting the iteration gets affected due to new loops
    std::vector<Loop*> nestedLoops;
    for (Loop *NL : *L) {
        nestedLoops.push_back(NL);
    }
    // Recursing over the original nested loops
    for (Loop *NL : nestedLoops) {
        getInnerMostLoops(NL, LI, DT, F, lsi);
    }
    // Splitting only if loop doesn't have nested loop
    if(nestedLoops.size() <= 0) {
        Value* loopIterator;
        LoopSplitInfo *LSI = new LoopSplitInfo(L);
        lsi->push_back(LSI);
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
    FPM.add(createPromoteMemoryToRegisterPass());
    FPM.run(F);

    // Iterating only on original loops and not the ones created
    std::vector<LoopSplitInfo*> lsi;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    for (Loop *L : LI) {
        getInnerMostLoops(L, &LI, &DT, &F, &lsi);
    }

    int maxTripCount = 0;
    int tripCount;
    std::vector<LoopSplitInfo*> valid_lsi;
    for(LoopSplitInfo *LSI : lsi) {
        if(IndirectAccessUtils::isLegalTransform(LSI->originalLoop)) {
            dbgs() << SE.getSmallConstantTripCount(LSI->originalLoop) << "\n";
            tripCount = SE.getSmallConstantTripCount(LSI->originalLoop);
            if(tripCount > maxTripCount) {
                maxTripCount = tripCount;
            }
            valid_lsi.push_back(LSI);
        }
    }
    dbgs() << maxTripCount << "\n";

    Value *array = IndirectAccessUtils::allocateArrayInEntryBlock(&F, maxTripCount);

    for(LoopSplitInfo *LSI : valid_lsi) {
        IndirectAccessUtils::clone(LSI, &LI, &DT);
        IndirectAccessUtils::clearClonedLoop(LSI);
        IndirectAccessUtils::populateArray(LSI, &LI, &DT, &F, array, &SE);
        IndirectAccessUtils::updateIndirectAccess(LSI, &F,array);
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
