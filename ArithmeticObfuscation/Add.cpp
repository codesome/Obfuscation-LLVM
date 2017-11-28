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
    // a ^ b
    Value* v_xor = Builder.CreateXor(a, b);
    // a & b
    Value* v_and = Builder.CreateAnd(a, b);
    Value* two = ConstantInt::get(type, 2);
    // 2 * (a & b)
    Value* v_mul = Builder.CreateMul(two, v_and);
    // (a ^ b) + 2 * (a & b)
    Value* final = Builder.CreateAdd(v_xor, v_mul);

    I->replaceAllUsesWith(final);

    return true;
}

Value* ifThenCaller(IRBuilder<>* ifThenBuilder, Type* floatType, 
    Value* aXX, Value* bXX, Value* aYY, Value* bYY, Value* aXXFloat, Value* bXXFloat) {

    // pInt = aXX + bXX
    Value *pInt = ifThenBuilder->CreateAdd(aXX, bXX);
    // pFloat = float(pInt) = float(aXX + bXX)
    Value *pFloat = ifThenBuilder->CreateSIToFP(pInt, floatType);
    // qFloat = aYY + bYY
    Value *qFloat = ifThenBuilder->CreateFAdd(aYY, bYY);
    // ifThenResult = pFloat + qFloat
    return ifThenBuilder->CreateFAdd(pFloat, qFloat);
}

Value* ifElseCaller(IRBuilder<>* ifElseBuilder, Value* a, Value* b){
    // a + b
    return ifElseBuilder->CreateFAdd(a,b);
}

bool obfuscateFloat(Instruction *I) {
    ArithmeticObfuscationUtils::floatObfuscator(I, 4611686018427387903.0, ifThenCaller, ifElseCaller);
    return true;
}

} /* namespace */

bool AddObfuscator::obfuscate(Instruction *I) {
    if(I->getOpcode() == Instruction::Add) {
        return obfuscateInteger(I);
    } else if (I->getOpcode() == Instruction::FAdd) {
        return obfuscateFloat(I);
    } else {
        return false;
    }
}
