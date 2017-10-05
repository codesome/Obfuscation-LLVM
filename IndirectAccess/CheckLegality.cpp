#include "llvm/Analysis/LoopInfo.h"
#include "IndirectAccess.h"
using namespace llvm;

bool CheckLegality::isLegalTransform(Loop *L) {
	return true;
}
