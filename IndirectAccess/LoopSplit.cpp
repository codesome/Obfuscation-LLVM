#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

LoopSplitInfo IndirectAccessUtils::splitAndCreateArray(Loop *L, int tripCount, LoopInfo *LI, DominatorTree *DT) {
	return LoopSplitInfo(nullptr,nullptr,nullptr);
}
