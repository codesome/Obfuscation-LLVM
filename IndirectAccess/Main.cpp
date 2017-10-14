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

/*___________________________________________________________
 *
 * Performs a DFS on the loops and populates the vector lsi
 * with the innermost loops
 *
 * @param Loop *L, the loop to be visited
 * @param std::vector<LoopSplitInfo*> *lsi, vector in which 
 *             innermost loops are pushed
 *___________________________________________________________*/
void getInnerMostLoops(Loop *L, std::vector<LoopSplitInfo*> *lsi) {
    bool containsNestedLoops = false;
    for (Loop *NL : *L) {
        containsNestedLoops = true;
        getInnerMostLoops(NL, lsi);
    }
    // Splitting only if loop doesn't have nested loop
    if(!containsNestedLoops) {
        LoopSplitInfo *LSI = new LoopSplitInfo(L);
        lsi->push_back(LSI);
    }
}

} /* namespace */

bool IndirectAccess::runOnFunction(Function &F) {
    bool modified = true;

    legacy::FunctionPassManager FPM(F.getParent());
    FPM.add(createLoopSimplifyPass());
    FPM.add(createPromoteMemoryToRegisterPass());
    FPM.run(F);

    LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

    // This vector will be filled with the inner most loops
    std::vector<LoopSplitInfo*> lsi;
    for (Loop *L : LI) {
        getInnerMostLoops(L, &lsi);
    }


    // This will contain the filtered innermost loops after validity check
    std::vector<LoopSplitInfo*> valid_lsi;
    // Max trip count of valid loops 
    int maxTripCount = 0;
    
    int tripCount;
    for(LoopSplitInfo *LSI : lsi) {
        if(IndirectAccessUtils::isLegalTransform(LSI->originalLoop)) {
            tripCount = SE.getSmallConstantTripCount(LSI->originalLoop);
            if(tripCount > maxTripCount) {
                maxTripCount = tripCount;
            }
            valid_lsi.push_back(LSI);
        }
    }

    // Allocating array of max trip count in entry block
    // This array will be reused in all the valid loops
    Value *array = IndirectAccessUtils::allocateArrayInEntryBlock(&F, maxTripCount);

    for(LoopSplitInfo *LSI : valid_lsi) {
        // clone the loop
        IndirectAccessUtils::clone(LSI, &LI, &DT);
        // clear the cloned loop
        IndirectAccessUtils::clearClonedLoop(LSI);
        // populate the array with induction variable in the cloned loop
        IndirectAccessUtils::populateArray(LSI, &F, array, &SE);
        // replace uses of insuction variable with indirect access in original loop
        IndirectAccessUtils::updateIndirectAccess(LSI, &F, array, &SE);
    }

    // dead instructions will be created while clearing cloned loop
    // hence removing it
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
