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
#include "IndirectAccess.h"
using namespace llvm;

#define DEBUG_TYPE "indirect-access"

namespace {

Value *temporaryIterator;

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
            // LSI->iterator = loopIterator;
            LSI->iterator = temporaryIterator;
            // temporaryIterator->dump();
            lsi->push_back(LSI);
            IndirectAccessUtils::cloneAndClearOriginal(LSI, LI, DT, F);
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
    // FPM.add(createLoopSimplifyPass());

    // ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    // temporary
    BasicBlock &HH = F.getEntryBlock();
    int cnt = 0;
    for(Instruction &I: HH) {
        cnt++;
        if(cnt==5) {
            temporaryIterator = &I;
            break;
        }
    }
    // temporary

    // Iterating only on original loops and not the ones created
    std::vector<LoopSplitInfo*> lsi;

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    // Splitting the original loops in function
    for (Loop *L : LI) {
        checkInnerMost(L, &LI, &DT, &F, &lsi);
    }
    FPM.add(createUnreachableBlockEliminationPass());
    FPM.run(F);

    for(LoopSplitInfo *LSI : lsi) {
        // Keeping the array size constant till we get a way to get 
        // trip count without causing problem in cloning and updating blocks
        LSI->tripCount = 1000;
        IndirectAccessUtils::initialiseAndUpdateArray(LSI, &LI, &DT, &F);
        IndirectAccessUtils::updateIndirectAccess(LSI, &F);
    }

    return modified;
}

void IndirectAccess::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    // AU.addRequired<ScalarEvolutionWrapperPass>();
}


// Registering the pass
char IndirectAccess::ID = 0;
static RegisterPass<IndirectAccess> X("indirect-access", "Indirect access of loop iterators");

#undef DEBUG_TYPE
