#include "llvm/IR/Function.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool DivObfuscator::obfuscate(Function *F) {
	bool modified = false;
	for(BasicBlock &BB : *F) {
		modified = modified || obfuscate(&BB);		
	}
    return modified;
}

bool DivObfuscator::obfuscate(BasicBlock *BB) {
	bool modified = false;
	for(Instruction &I : *BB) {
		modified = modified || obfuscate(&I);
	}
    return modified;
}

bool DivObfuscator::obfuscate(Instruction *I) {
    return false;
}
