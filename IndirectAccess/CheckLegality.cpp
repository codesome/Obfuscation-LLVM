#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess/IndirectAccess.h"
using namespace llvm;

bool IndirectAccessUtils::isLegalTransform(Loop *L, ScalarEvolution *SE) {
	Value *iterator = IndirectAccessUtils::getIntegerIterator(L,SE);
	return SE->getSmallConstantTripCount(L) > 0
		&& iterator!=nullptr 
		&& iterator->getType()->getPrimitiveSizeInBits() <= IndirectAccessUtils::MAX_BITS;
}
