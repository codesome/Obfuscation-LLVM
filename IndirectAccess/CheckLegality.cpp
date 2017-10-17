#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

bool IndirectAccessUtils::isLegalTransform(Loop *L, ScalarEvolution *SE) {
	return SE->getSmallConstantTripCount(L)>0 && IndirectAccessUtils::getIterator(L,SE)!=nullptr;
}
