#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "ConstantsEncoding.h"
using namespace llvm;

namespace {

void populateBody(IRBuilder<>* loopBodyBuilder, LLVMContext& context, 
    GlobalVariable *globalString, Value *iterAlloca, Value *newStrAlloca, int offset) {

    Type* i8 = Type::getInt8Ty(context);
    Value* zero = ConstantInt::get(Type::getInt32Ty(context), 0);
    Value* v126 = ConstantInt::get(i8, 126);
    Value* v127 = ConstantInt::get(i8, 127);
    Value* offsetValue = ConstantInt::get(i8, offset);
    
    Value *iterBodyLoad = loopBodyBuilder->CreateLoad(iterAlloca);
    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(iterBodyLoad);
    ArrayRef<Value*> idxListBody(idxVector);
    Value *bodyGlobalVarGEP = loopBodyBuilder->CreateInBoundsGEP(globalString, idxListBody);
    Value *bodyNewStrGEP = loopBodyBuilder->CreateInBoundsGEP(newStrAlloca, idxListBody);
    
    Value *ascii = loopBodyBuilder->CreateLoad(bodyGlobalVarGEP);
    Value *add1 = loopBodyBuilder->CreateAdd(ascii, v126);
    Value *rem1 = loopBodyBuilder->CreateURem(add1, v127);
    Value *sub = loopBodyBuilder->CreateSub(rem1, offsetValue);
    Value *add2 = loopBodyBuilder->CreateAdd(sub, v127);
    Value *rem2 = loopBodyBuilder->CreateURem(add2, v127);
    loopBodyBuilder->CreateStore(rem2, bodyNewStrGEP);
}

Value* populateLatch(IRBuilder<>* loopLatchBuilder, Value *iterAlloca, Value *loopBound, Value *one) {
    Value *iterLoad = loopLatchBuilder->CreateLoad(iterAlloca);
    Value *iterIncr = loopLatchBuilder->CreateAdd(iterLoad, one);
    loopLatchBuilder->CreateStore(iterIncr, iterAlloca);
    return loopLatchBuilder->CreateICmpSLT(iterIncr, loopBound);
}

Value* populateEnd(IRBuilder<>* loopEndBuilder, GlobalVariable *globalString, 
    Value *newStrAlloca, Value *zero, Value *loopBound) {
    
    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(loopBound);
    ArrayRef<Value*> idxListEnd1(idxVector);
    Value *bodyGlobalVarGEP = loopEndBuilder->CreateInBoundsGEP(globalString, idxListEnd1);
    Value *bodyNewStrGEP = loopEndBuilder->CreateInBoundsGEP(newStrAlloca, idxListEnd1);
    Value *ascii = loopEndBuilder->CreateLoad(bodyGlobalVarGEP);
    loopEndBuilder->CreateStore(ascii, bodyNewStrGEP);
    
    idxVector.clear();
    idxVector.push_back(zero);
    idxVector.push_back(zero);
    ArrayRef<Value*> idxListEnd2(idxVector);
    return loopEndBuilder->CreateInBoundsGEP(newStrAlloca, idxListEnd2);
}

void inlineDecode(GlobalVariable *globalString, Value *v, Instruction *I, int offset) {

    static LLVMContext& context = globalString->getContext();
    Constant *constString = globalString->getInitializer();
    ConstantDataArray *csa = cast<ConstantDataArray>(constString);
    std::string str = csa->getAsCString();
    int stringLength = str.length();
    Type* i32 = Type::getInt32Ty(context);
    Value* zero = ConstantInt::get(i32, 0);
    Value* one = ConstantInt::get(i32, 1);
    Value* loopBound = ConstantInt::get(i32, stringLength);
    
    Function *F = I->getParent()->getParent();
    BasicBlock* loopHeader = BasicBlock::Create(context,"for.head",F);
    BasicBlock* loopBody = BasicBlock::Create(context,"for.body",F);
    BasicBlock* loopLatch = BasicBlock::Create(context,"for.inc",F);
    BasicBlock* loopEnd = BasicBlock::Create(context,"for.end",F);
    Instruction* toMoveInst = I->getNextNode();
    IRBuilder<> builder(I);
    builder.CreateBr(loopHeader);

    // HEADER
    IRBuilder<> loopHeaderBuilder(loopHeader);
    PointerType *pType = globalString->getType();
    Value *newStrAlloca = loopHeaderBuilder.CreateAlloca(pType->getElementType());
    Value *iterAlloca = loopHeaderBuilder.CreateAlloca(i32);
    loopHeaderBuilder.CreateStore(zero, iterAlloca);
    loopHeaderBuilder.CreateBr(loopBody);

    // BODY
    IRBuilder<> loopBodyBuilder(loopBody);
    populateBody(&loopBodyBuilder, context, globalString, iterAlloca, newStrAlloca, offset);
    loopBodyBuilder.CreateBr(loopLatch);

    // LATCH
    IRBuilder<> loopLatchBuilder(loopLatch);
    Value* cond = populateLatch(&loopLatchBuilder, iterAlloca, loopBound, one);
    loopLatchBuilder.CreateCondBr(cond, loopBody, loopEnd);

    // END
    IRBuilder<> loopEndBuilder(loopEnd);
    Value *newStrGEP = populateEnd(&loopEndBuilder, globalString, newStrAlloca, zero, loopBound);

    Instruction* next = dyn_cast<Instruction>(newStrGEP);
    I->removeFromParent();
    I->insertAfter(next);
    next = I;
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
    v->replaceAllUsesWith(newStrGEP);
}


} /* namespace */

void ConstantsEncodingUtils::decode(GlobalVariable* globalString, int offset) {

    if(globalString->isConstant() && globalString->hasInitializer()) {
        for(User *U: globalString->users()) {
            if(Value *val = dyn_cast<Value>(U)) {
                Instruction *I = dyn_cast<Instruction>(U);
                if(!I) {
                    for(User *uu: val->users()) {
                        I = dyn_cast<Instruction>(uu);
                        if(I) break;
                    }
                }
                if(!I) continue;

                inlineDecode(globalString, val, I, offset);

            }
        }
    }

}
