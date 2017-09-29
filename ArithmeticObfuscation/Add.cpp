#include "llvm/IR/Function.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool AddObfuscator::obfuscate(Function *F) {
	bool modified = false;
	for(BasicBlock &BB : *F) {
		modified = modified || obfuscate(&BB);		
	}
    return modified;
}

bool AddObfuscator::obfuscate(BasicBlock *BB) {
	bool modified = false;
	for(Instruction &I : *BB) {
		modified = modified || obfuscate(&I);
	}
    return modified;
}

bool AddObfuscator::obfuscate(Instruction *I) {
    return false;
}
