#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool SubObfuscator::obfuscate(Function *F) {
	bool modified = false;
    for(BasicBlock &BB : *F) {
        modified = obfuscate(&BB) || modified;        
    }
    return modified;
}

bool SubObfuscator::obfuscate(BasicBlock *BB) {
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

bool SubObfuscator::obfuscate(Instruction *I) {
    if(I->getOpcode() != Instruction::Sub)
        return false;

    Type* type = I->getType();
    if(!type->isIntegerTy())
        return false;

    Value* a = I->getOperand(0);
    Value* b = I->getOperand(1);

    IRBuilder<> Builder(I);
    // ~b
    Value* v_not = Builder.CreateNot(b);
    Value* one = ConstantInt::get(type, 1);
    // ~b + 1
    Value* v_add = Builder.CreateAdd(one, v_not);
    // a + ~b + 1
    Value* final = Builder.CreateAdd(v_add, a);

    I->replaceAllUsesWith(final);

    return true;
}
