#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ConstantsEncoding.h"
using namespace llvm;

#define DEBUG_TYPE "const-encoding"

bool ConstantsEncoding::runOnFunction(Function &F) {
    return true;
}

// Registering the pass
char ConstantsEncoding::ID = 0;
static RegisterPass<ConstantsEncoding> X("const-encoding", "Obfuscates string constants");

#undef DEBUG_TYPE
