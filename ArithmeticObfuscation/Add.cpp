#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool AddObfuscator::obfuscate(Function *F) {
    bool modified = false;
    for(BasicBlock &BB : *F) {
        modified = obfuscate(&BB) || modified;        
    }
    return modified;
}

bool AddObfuscator::obfuscate(BasicBlock *BB) {
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

bool AddObfuscator::obfuscate(Instruction *I) {
    if(I->getOpcode() != Instruction::Add)
        return false;

    Type* type = I->getType();
    if(!type->isIntegerTy())
        return false;

    Value* a = I->getOperand(0);
    Value* b = I->getOperand(1);

    IRBuilder<> Builder(I);
    Value* v_xor = Builder.CreateXor(a, b);
    Value* v_and = Builder.CreateAnd(a, b);
    Value* two = ConstantInt::get(type, 2);
    Value* v_mul = Builder.CreateMul(two, v_and);
    Value* final = Builder.CreateAdd(v_xor, v_mul);

    I->replaceAllUsesWith(final);

    return true;
}
