#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool MulObfuscator::obfuscate(Function *F) {
	bool modified = false;
    for(BasicBlock &BB : *F) {
        modified = obfuscate(&BB) || modified;        
    }
    return modified;
}

bool MulObfuscator::obfuscate(BasicBlock *BB) {
	bool modified = false;
    std::vector<Instruction *> toErase;
    for(Instruction &I : *BB) {
        if(obfuscate(&I)) {
            modified = true;
            toErase.push_back(&I);
        }
    }
    for(Instruction *I: toErase) {
        I->eraseFromParent();
    }

    return modified;
}

bool MulObfuscator::obfuscate(Instruction *I) {
    return false;
}
