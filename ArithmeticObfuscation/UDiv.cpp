#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

bool UDivObfuscator::obfuscate(Instruction *I) {
     if(I->getOpcode() != Instruction::UDiv)
        return false;

    Type* type = I->getType();
    if(!type->isIntegerTy())
        return false;

    // Getting Dividend and Divisor 
    Value* Dividend = I->getOperand(0);
    Value* Divisor = I->getOperand(1);

    IRBuilder<> Builder(I);


    // Reminder = Dividend % divisor
    Value *Remainder = Builder.CreateURem(Dividend, Divisor);

    Value* minusone = ConstantInt::get(type, -1);
    Value *minusRemainder = Builder.CreateMul(Remainder, minusone);

    // Dividend - Remainder 
    Value *numerator = Builder.CreateAdd(Dividend,minusRemainder);

    // (Dividend - Remainder)/Divisor
    Value* final = Builder.CreateUDiv(numerator,Divisor);

    I->replaceAllUsesWith(final);
    return true;
}
