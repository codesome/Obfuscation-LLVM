#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

void IndirectAccessUtils::updateIndirectAccess(LoopSplitInfo* LSI) {
	if(LSI->clonedLoop==nullptr)
		return;
	
    Loop* L = LSI->clonedLoop;
    BasicBlock *preHeader = L->getLoopPreheader();
    BasicBlock *loopLatch = L->getLoopLatch();

    for(BasicBlock *B : L->getBlocks()) {
    	if(B!=preHeader && B!=loopLatch) {
	    	// blocks which are not header and not latch
    	}
    }
}
