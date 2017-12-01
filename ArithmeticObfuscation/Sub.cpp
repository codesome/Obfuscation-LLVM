#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "ArithmeticObfuscation/ArithmeticObfuscation.h"
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

Value* ifThenCaller(IRBuilder<>* ifThenBuilder, Type* floatType, 
    Value* aXX, Value* bXX, Value* aYY, Value* bYY, Value* aXXFloat, Value* bXXFloat) {
    // pInt = aXX - bXX
    Value *pInt = ifThenBuilder->CreateSub(aXX, bXX);
    // pFloat = int64(pInt) = int64(aXX - bXX)
    Value *pFloat = ifThenBuilder->CreateSIToFP(pInt, floatType);
    // qFloat = aYY - bYY
    Value *qFloat = ifThenBuilder->CreateFSub(aYY, bYY);
    // ifThenResult = pFloat + qFloat
    return ifThenBuilder->CreateFAdd(pFloat, qFloat);
}

Value* ifElseCaller(IRBuilder<>* ifElseBuilder, Value* a, Value* b){
    // a - b
    return ifElseBuilder->CreateFSub(a,b);      
}


bool obfuscateFloat(Instruction *I) {
    ArithmeticObfuscationUtils::floatObfuscator(I, 4611686018427387903.0, ifThenCaller, ifElseCaller);
    return true;
}

} /* namespace */

bool SubObfuscator::obfuscate(Instruction *I) {
    if(I->getOpcode() == Instruction::Sub) {
        return obfuscateInteger(I);
    } else if (I->getOpcode() == Instruction::FSub) {
        return obfuscateFloat(I);
    } else {
        return false;
    }
}
