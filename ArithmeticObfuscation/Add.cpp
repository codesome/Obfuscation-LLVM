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

bool obfuscateFloat(Instruction *I) {
    Value* a = I->getOperand(0);
    Value* b = I->getOperand(1);
    static LLVMContext& context = I->getParent()->getContext();

    // type of float of this instruction
    Type* floatType = a->getType();

    // i64 of LLVM
    Type* i64 = Type::getInt64Ty(context);

    // (INT_MAX for 64 bits) / 2
    double INT_MAX64_DIV_2 = 4611686018427387903.0;
    Constant* INT_MAX64_DIV_2_FP = ConstantFP::get(floatType, INT_MAX64_DIV_2);
    
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
    Value* aCond = conditionBuilder.CreateFCmpOLT(a, INT_MAX64_DIV_2_FP);
    Value* bCond = conditionBuilder.CreateFCmpOLT(b, INT_MAX64_DIV_2_FP);
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
    // pInt = aXX + bXX
    Value *pInt = ifThenBuilder.CreateAdd(aXX, bXX);
    // pFloat = int64(pInt) = int64(aXX + bXX)
    Value *pFloat = ifThenBuilder.CreateSIToFP(pInt, floatType);
    // qFloat = aYY + bYY
    Value *qFloat = ifThenBuilder.CreateFAdd(aYY, bYY);
    // ifThenResult = pFloat + qFloat
    Value *ifThenResult = ifThenBuilder.CreateFAdd(pFloat, qFloat);
    ifThenBuilder.CreateStore(ifThenResult, result);
    ifThenBuilder.CreateBr(ifEndBB);
    
    /** if.else **/
    IRBuilder<> ifElseBuilder(ifElseBB);
    // a + b
    Value* ifElseResult = ifElseBuilder.CreateFAdd(a,b);
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

    return true;
}

} /* namespace */

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
    if(I->getOpcode() == Instruction::Add) {
        return obfuscateInteger(I);
    } else if (I->getOpcode() == Instruction::FAdd) {
        return obfuscateFloat(I);
    } else {
        return false;
    }
}
