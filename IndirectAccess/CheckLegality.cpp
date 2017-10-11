#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

bool IndirectAccessUtils::isLegalTransform(Loop *L) {
	return true;
}
