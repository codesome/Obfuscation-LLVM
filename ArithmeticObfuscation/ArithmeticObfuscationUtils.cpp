#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "ArithmeticObfuscation.h"

void ArithmeticObfuscationUtils::floatObfuscator(
        Instruction *I,
        double maxAllowedValue,
        Value* (*ifThenCaller)(IRBuilder<>*, Type*, Value*, Value*, Value*, Value*, Value*, Value*), 
        Value* (*ifElseCaller)(IRBuilder<>*, Value*, Value*)) {

    Value* a = I->getOperand(0);
    Value* b = I->getOperand(1);
    static LLVMContext& context = I->getParent()->getContext();

    // type of float of this instruction
    Type* floatType = a->getType();

    // i64 of LLVM
    Type* i64 = Type::getInt64Ty(context);

    Constant* maxAllowedValueConst = ConstantFP::get(floatType, maxAllowedValue);
    
    // from this instructions onwards, all the instructions 
    // till break will be be moved to if.end block
    Instruction* toMoveInst = I->getNextNode();

    // contains the obfuscated FAdd
    BasicBlock* ifThenBB = BasicBlock::Create(context,"if.then",I->getParent()->getParent());
    // Normal FAdd (a+b)
    BasicBlock* ifElseBB = BasicBlock::Create(context,"if.else",I->getParent()->getParent());
    // Takes result of if.then or if.else and replaces with original instruction
    BasicBlock* ifEndBB = BasicBlock::Create(context,"if.end",I->getParent()->getParent());
    
    // if(a<INT_MAX64_DIV_2_FP && b<INT_MAX64_DIV_2_FP)
    // then obfuscate, a+b can never overflow i64
    // else no, because a+b can overflow i64
    IRBuilder<> conditionBuilder(I);
    Value* aCond = conditionBuilder.CreateFCmpOLT(a, maxAllowedValueConst);
    Value* bCond = conditionBuilder.CreateFCmpOLT(b, maxAllowedValueConst);
    Value* ifcond = conditionBuilder.CreateAnd(aCond, bCond);
    Value* result = conditionBuilder.CreateAlloca(floatType);
    conditionBuilder.CreateCondBr(ifcond, ifThenBB, ifElseBB);

    /** if.then **/
    IRBuilder<> ifThenBuilder(ifThenBB);
    // aXX = int64(a)
    Value *aXX = ifThenBuilder.CreateFPToSI(a, i64);
    // aXXFloat = float(aXX) = float(int64(a))
    Value *aXXFloat = ifThenBuilder.CreateSIToFP(aXX, floatType);
    // bXX = int64(b)
    Value *bXX = ifThenBuilder.CreateFPToSI(b, i64);
    // bXXFloat = float(bXX) = float(int64(b))
    Value *bXXFloat = ifThenBuilder.CreateSIToFP(bXX, floatType);

    // aYY = a - aXXFloat = a - float(int64(a))
    Value *aYY = ifThenBuilder.CreateFSub(a,aXXFloat);
    // bYY = b - bXXFloat = b - float(int64(b))
    Value *bYY = ifThenBuilder.CreateFSub(b,bXXFloat);
    
    Value *ifThenResult = ifThenCaller(&ifThenBuilder, floatType, aXX, bXX, aYY, bYY, aXXFloat, bXXFloat);
    ifThenBuilder.CreateStore(ifThenResult, result);
    ifThenBuilder.CreateBr(ifEndBB);
    
    /** if.else **/
    IRBuilder<> ifElseBuilder(ifElseBB);
    Value* ifElseResult = ifElseCaller(&ifElseBuilder, a, b);
    ifElseBuilder.CreateStore(ifElseResult, result);
    ifElseBuilder.CreateBr(ifEndBB);

    /** if.end **/
    IRBuilder<> ifEndBuilder(ifEndBB);
    Value *resultLoad = ifEndBuilder.CreateLoad(result);

    // moving all instruction after I from original block
    // to if.end, after the phi instruction
    Instruction* next = dyn_cast<Instruction>(resultLoad);
    std::vector<Instruction *> toMove;
    while(toMoveInst != nullptr) {
        toMove.push_back(toMoveInst);
        toMoveInst = toMoveInst->getNextNode();
    }
    for(Instruction *II: toMove) {
            II->removeFromParent();
            II->insertAfter(next);
            next = II;
    }

    I->replaceAllUsesWith(resultLoad);

}
