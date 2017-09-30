#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

#define DEBUG_TYPE "arith-obfus"

bool ArithmeticObfuscation::runOnFunction(Function &F) {
    bool modified = false;
    // TODO: decide the order and number of times to run obfuscate functions
    modified = SubObfuscator::obfuscate(&F) || modified;
    modified = AddObfuscator::obfuscate(&F) || modified;
    modified = MulObfuscator::obfuscate(&F) || modified;
    modified = DivObfuscator::obfuscate(&F) || modified;
    
    return modified;
}

void ArithmeticObfuscation::getAnalysisUsage(AnalysisUsage &AU) const {

}


// Registering the pass
char ArithmeticObfuscation::ID = 0;
static RegisterPass<ArithmeticObfuscation> X("arith-obfus", "Obfuscates arithmetic operations");

#undef DEBUG_TYPE