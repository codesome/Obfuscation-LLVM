#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

namespace {

bool obfuscateInteger(Instruction *I) {
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

bool obfuscateFloat(Instruction *I) {

    // Value* a = I->getOperand(0);
    // Value* b = I->getOperand(1);

    /* TODO:
        1) Add if else to check if floor(a)<=MAX_INT64 and floor(b)<=MAX_INT64, [check for NaN too?]
        2) If true, obfuscate in if-then
        3) If false, use fadd in else

        // original inst will be removed
        // float can also be double
    */



    /* if-then
        aXX = bitcast float a to i64
        aXXFloat = bitcast i64 aXX to float
        aYY = a - aXXFloat

        bXX = bitcast float b to i64
        bXXFloat = bitcast i64 bXX to float
        bYY = b - bXXFloat

        p = aXXFloat - bYY
        q = aYY - bXXFloat

        r = p + q

        replace I with r
    */

    /* else
        r = a - b
        replace I with r
    */

    /* remove I */

    return false;
}

} /* namespace */

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
    if(I->getOpcode() == Instruction::Add) {
        return obfuscateInteger(I);
    } else if (I->getOpcode() == Instruction::FAdd) {
        return obfuscateFloat(I);
    } else {
        return false;
    }
}
