#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "ConstantEncoding/ConstantEncoding.h"
using namespace llvm;

namespace {

/*___________________________________________________________________
 *
 * Caesar cipher
 * Populates the body of loop used in decode
 *
 * @param IRBuilder<>* loopBodyBuilder, the builder of the body
 * @param LLVMContext& context, as the name says
 * @param GlobalVariable *globalVar, the variable to be decoded
 * @param Value *iterAlloca, the register allocated as loop iterator
 * @param Value *newAlloca, allocated for decoded result
 * @param int offset, offset used while encoding
 * @param int unused, this is an unused parameter
 *___________________________________________________________________*/
void populateBodyCaesar(IRBuilder<>* loopBodyBuilder, LLVMContext& context, 
    GlobalVariable *globalVar, Value *iterAlloca, Value *newAlloca, int offset, int unused) {

    // Values needed
    Type* i8 = Type::getInt8Ty(context);
    Value* zero = ConstantInt::get(Type::getInt32Ty(context), 0);
    Value* v126 = ConstantInt::get(i8, 126);
    Value* v127 = ConstantInt::get(i8, 127);
    Value* offsetValue = ConstantInt::get(i8, offset);

    // getting character at index=iterator
    Value *iterBodyLoad = loopBodyBuilder->CreateLoad(iterAlloca);
    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(iterBodyLoad);
    ArrayRef<Value*> idxListBody(idxVector);
    // character from global variable
    Value *globalVarGEP = loopBodyBuilder->CreateInBoundsGEP(globalVar, idxListBody);
    // character from new string
    Value *newStrGEP = loopBodyBuilder->CreateInBoundsGEP(newAlloca, idxListBody);
    
    // ascii value of global variable character
    Value *ascii = loopBodyBuilder->CreateLoad(globalVarGEP);
    // ascii + 126
    Value *add1 = loopBodyBuilder->CreateAdd(ascii, v126);
    // (ascii + 126)%127
    Value *rem1 = loopBodyBuilder->CreateURem(add1, v127);
    // (ascii + 126)%127 - offset
    Value *sub = loopBodyBuilder->CreateSub(rem1, offsetValue);
    // (ascii + 126)%127 - offset + 127
    Value *add2 = loopBodyBuilder->CreateAdd(sub, v127);
    // ((ascii + 126)%127 - offset + 127)%127
    Value *rem2 = loopBodyBuilder->CreateURem(add2, v127);
    // decoded_character = ((ascii + 126)%127 - offset + 127)%127
    loopBodyBuilder->CreateStore(rem2, newStrGEP);
}

/*___________________________________________________________________
 *
 * Bit encoding and decoding
 * Populates the body of loop used in decode.
 *
 * @param IRBuilder<>* loopBodyBuilder, the builder of the body
 * @param LLVMContext& context, as the name says
 * @param GlobalVariable *globalVar, the variable to be decoded
 * @param Value *iterAlloca, the register allocated as loop iterator
 * @param Value *newAlloca, allocated for decoded result
 * @param int nBits, number of bits encoded in each character
 * @param int unused, this is an unused parameter
 *___________________________________________________________________*/
void populateBodyBitEncodingAndDecoding(IRBuilder<>* loopBodyBuilder, LLVMContext& context, 
    GlobalVariable *globalVar, Value *iterAlloca, Value *newAlloca, int nBits, int unused) {

    // Values needed
    int nChar = 8/nBits;
    Type* i8 = Type::getInt8Ty(context);
    Type* i32 = Type::getInt32Ty(context);
    Value* step = ConstantInt::get(i32, nChar);
    Value* zeroi8 = ConstantInt::get(i8, 0);
    Value* zero = ConstantInt::get(i32, 0);
    Value* one = ConstantInt::get(i32, 1);

    int mask = 1;
    for(int i=1; i<nBits; i++) {
        mask = (mask<<1) + 1;
    }
    Value* maskVal = ConstantInt::get(i8, mask);

    // getting character at index=iterator
    Value *iterLoad = loopBodyBuilder->CreateLoad(iterAlloca);
    Value *newStringIndex = loopBodyBuilder->CreateSDiv(iterLoad, step);

    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(newStringIndex);
    ArrayRef<Value*> idxListBody(idxVector);
    // character from new string
    Value *newStrGEP = loopBodyBuilder->CreateInBoundsGEP(newAlloca, idxListBody);
    loopBodyBuilder->CreateStore(zeroi8, newStrGEP);
    Value *newStrLoad = loopBodyBuilder->CreateLoad(newStrGEP);

    for(int i=0; i<nChar; i++) {
        idxVector.clear();
        idxVector.push_back(zero);
        idxVector.push_back(iterLoad);
        // character from global variable
        Value *globalVarGEP = loopBodyBuilder->CreateInBoundsGEP(globalVar, idxListBody);
        Value *globalVarLoad = loopBodyBuilder->CreateLoad(globalVarGEP);
        Value *orVal = loopBodyBuilder->CreateAnd(globalVarLoad, maskVal);
        Value *shift = loopBodyBuilder->CreateShl(orVal, ConstantInt::get(i8, i*nBits));
        newStrLoad = loopBodyBuilder->CreateAdd(newStrLoad, shift);
        if(i!=nChar-1) {
            iterLoad = loopBodyBuilder->CreateAdd(iterLoad, one);
        }
    }

    loopBodyBuilder->CreateStore(newStrLoad, newStrGEP);

}

/*___________________________________________________________________
 *
 * Bit encoding and decoding
 * Populates the body of loop used in decode.
 *
 * @param IRBuilder<>* loopBodyBuilder, the builder of the body
 * @param LLVMContext& context, as the name says
 * @param GlobalVariable *globalVar, the variable to be decoded
 * @param Value *iterAlloca, the register allocated as loop iterator
 * @param Value *newAlloca, allocated for decoded result
 * @param int nBits, number of bits encoded in each character
 * @param int integerBits, number of bits in the original integer
 *___________________________________________________________________*/
void populateBodyBitEncodingAndDecodingNumbers(IRBuilder<>* loopBodyBuilder, LLVMContext& context, 
    GlobalVariable *globalVar, Value *iterAlloca, Value *newAlloca, int nBits, int integerBits) {

    // Values needed
    Type* iN = Type::getIntNTy(context, integerBits);
    Type* i32 = Type::getInt32Ty(context);
    Value* zero = ConstantInt::get(i32, 0);

    int mask = 1;
    for(int i=1; i<nBits; i++) {
        mask = (mask<<1) + 1;
    }
    Value* maskVal = ConstantInt::get(Type::getInt8Ty(context), mask);

    // getting character at index=iterator
    Value *iterLoad = loopBodyBuilder->CreateLoad(iterAlloca);

    std::vector<Value*> idxVector;
    idxVector.push_back(zero);
    idxVector.push_back(iterLoad);
    ArrayRef<Value*> idxListBody(idxVector);
    Value *globalVarGEP = loopBodyBuilder->CreateInBoundsGEP(globalVar, idxListBody);
    Value *globalVarLoad = loopBodyBuilder->CreateLoad(globalVarGEP);
    Value *newAllocaLoad = loopBodyBuilder->CreateLoad(newAlloca);

    Value *orVal = loopBodyBuilder->CreateAnd(globalVarLoad, maskVal);    
    if(orVal->getType()->getIntegerBitWidth() != (unsigned int)integerBits) {
        orVal = loopBodyBuilder->CreateSExtOrTrunc(orVal, iN);
    }
    if(iterLoad->getType()->getIntegerBitWidth() != (unsigned int)integerBits) {
        iterLoad = loopBodyBuilder->CreateSExtOrTrunc(iterLoad, iN);
    }

    Value *mul = loopBodyBuilder->CreateMul(iterLoad, ConstantInt::get(iN, nBits));
    Value *shift = loopBodyBuilder->CreateShl(orVal, mul);
    Value *newAllocaAdd = loopBodyBuilder->CreateAdd(newAllocaLoad, shift);
    loopBodyBuilder->CreateStore(newAllocaAdd, newAlloca);
    
}

/*___________________________________________________________________
 *
 * Builds the conditions in the loop latch of decoding loop
 *
 * @param IRBuilder<>* loopLatchBuilder, the builder for loop latch
 * @param Value *iterAlloca, the register allocated as loop iterator
 * @param Value *loopBound, Value* for string length 
 * @param Value *iterStep, Value* for increment of iterator
 * NOTE: type of iterAlloca, loopBound and one should be same
 *___________________________________________________________________*/
Value* populateLatch(IRBuilder<>* loopLatchBuilder, Value *iterAlloca, Value *loopBound, Value *iterStep) {
    // iterator++
    Value *iterLoad = loopLatchBuilder->CreateLoad(iterAlloca);
    Value *iterIncr = loopLatchBuilder->CreateAdd(iterLoad, iterStep);
    loopLatchBuilder->CreateStore(iterIncr, iterAlloca);
    // iterator < loopBound
    return loopLatchBuilder->CreateICmpSLT(iterIncr, loopBound);
}

/*___________________________________________________________________
 *
 * Add null character at the end of decoded string
 *
 * @param bool isNumber, true if decoded result is a number, 
 *                       false if string
 * @param IRBuilder<>* loopEndBuilder, the builder for loop end
 * @param GlobalVariable *globalVar, the variable to be decoded
 * @param Value *newAlloca, allocated for decoded result
 * @param Value *zero, Value* for 0
 * @param Value *loopBound, Value* for string length 
 * @param Value *newStrEnd, last index for decoded string 
 * NOTE: type of loopBound and zero should be same
 * @return Value*, the decoded string/number (i8* or iN)
 *___________________________________________________________________*/
Value* populateEnd(bool isNumber, IRBuilder<>* loopEndBuilder, GlobalVariable *globalVar, 
    Value *newAlloca, Value *zero, Value *loopBound, Value *newStrEnd) {
    if(isNumber) {
        return loopEndBuilder->CreateLoad(newAlloca);
    } else {
        // copying last character from string, usually null
        // this should not be decoded
        std::vector<Value*> idxVector;
        idxVector.push_back(zero);
        idxVector.push_back(loopBound);
        ArrayRef<Value*> idxListEndGlobal(idxVector);
        // Encoded string last character
        Value *globalVarGEP = loopEndBuilder->CreateInBoundsGEP(globalVar, idxListEndGlobal);
        Value *ascii = loopEndBuilder->CreateLoad(globalVarGEP);

        idxVector.clear();
        idxVector.push_back(zero);
        idxVector.push_back(newStrEnd);
        ArrayRef<Value*> idxListEndNewStr(idxVector);
        // new string last character
        Value *newStrGEP = loopEndBuilder->CreateInBoundsGEP(newAlloca, idxListEndNewStr);
        
        loopEndBuilder->CreateStore(ascii, newStrGEP);
        
        // getting the string from the allocated variable
        idxVector.clear();
        idxVector.push_back(zero);
        idxVector.push_back(zero);
        ArrayRef<Value*> idxListEnd2(idxVector);
        return loopEndBuilder->CreateInBoundsGEP(newAlloca, idxListEnd2);
    }
}

/*___________________________________________________________________________
 *
 * Inlines decode algorithm in IR
 *
 * @param GlobalVariable *globalVar, the encoded variable
 * @param int *loopBoundInt, the encoded string length
 * @param int *loopIterStep, iterator+=loopIterStep in loop latch
 * @param Value *originalValue, the original use of encoded string 
 *              in the IR which is to be replaced with decoded value
 * @param Instruction *I, instruction just before the use of originalValue 
 *  (if value is part of instruction), else the originalValue as Instruction.
 *        Decode algorithm is built just before this instruction.
 * @param int param, offset (or) nBits used while encoding
 * @param (*populateBody), function used to populate the decode loop body.
 *              For aruments of function, check populateBody functions above
 * @param GlobalVariable *newStringVar, new encoded variable created
 *        if this is given, it is considered as encoded variable
 *        and globalVar will be deleted from IR
 *___________________________________________________________________________*/
void inlineDecode(bool isCaesar, bool isNumber, GlobalVariable *globalVar, int loopBoundInt, 
    int loopIterStep, Value *originalValue, Instruction *I, int param,
    void (*populateBody)(IRBuilder<>*,LLVMContext&,GlobalVariable*,Value*,Value*,int,int), 
    GlobalVariable *newStringVar=nullptr, int integerBits=0, LLVMContext *ctx=nullptr) {

    GlobalVariable *encodedGlobalVar = newStringVar==nullptr? globalVar: newStringVar;

    static LLVMContext& context = ctx==nullptr? globalVar->getContext() : *ctx;
    // Values required later
    Type *i32 = Type::getInt32Ty(context);
    Value* zero = ConstantInt::get(i32, 0);
    Value* iterStep = ConstantInt::get(i32, loopIterStep);
    Value* loopBound = ConstantInt::get(i32, loopBoundInt);
    
    // Creating new basic blocks for the loop
    Function *F = I->getParent()->getParent();
    // new block before the cycle for loop
    BasicBlock* loopHeader = BasicBlock::Create(context,"for.head",F);
    // main loop body which will hold decode logic
    BasicBlock* loopBody = BasicBlock::Create(context,"for.body",F);
    // loop latch of loop, takes from i=0 -> i=stringLength-1
    BasicBlock* loopLatch = BasicBlock::Create(context,"for.inc",F);
    // Block to clean up some parts of decode and replace uses
    BasicBlock* loopEnd = BasicBlock::Create(context,"for.end",F);

    // from this instruction, everything has to be moved to for.end
    Instruction* toMoveInst = I->getNextNode();

    // break to the loop
    IRBuilder<> builder(I);
    builder.CreateBr(loopHeader);

    // HEADER
    IRBuilder<> loopHeaderBuilder(loopHeader);
    // allocating new string for the decode result
    Value *newAlloca;
    if(isNumber) {
        Type *iN = Type::getIntNTy(context, integerBits);
        newAlloca = loopHeaderBuilder.CreateAlloca(iN);
        loopHeaderBuilder.CreateStore(ConstantInt::get(iN, 0), newAlloca);
    } else if(isCaesar) {
        PointerType *pType = encodedGlobalVar->getType();
        newAlloca = loopHeaderBuilder.CreateAlloca(pType->getElementType());
    } else {
        newAlloca = loopHeaderBuilder.CreateAlloca(ArrayType::get(Type::getInt8Ty(context), (loopBoundInt/loopIterStep)+1));
    }
    // loop iterator, goes from 0 to stringLength-1, with steps of loopIterStep
    Value *iterAlloca = loopHeaderBuilder.CreateAlloca(i32);
    loopHeaderBuilder.CreateStore(zero, iterAlloca);
    loopHeaderBuilder.CreateBr(loopBody);

    // BODY
    IRBuilder<> loopBodyBuilder(loopBody);
    populateBody(&loopBodyBuilder, context, encodedGlobalVar, iterAlloca, newAlloca, param, integerBits);
    loopBodyBuilder.CreateBr(loopLatch);

    // LATCH
    IRBuilder<> loopLatchBuilder(loopLatch);
    Value* cond = populateLatch(&loopLatchBuilder, iterAlloca, loopBound, iterStep);
    loopLatchBuilder.CreateCondBr(cond, loopBody, loopEnd);

    // END
    IRBuilder<> loopEndBuilder(loopEnd);
    Value *newStrGEP;
    if(isCaesar) {
        newStrGEP = populateEnd(isNumber, &loopEndBuilder, encodedGlobalVar, newAlloca, zero, loopBound, loopBound);
    } else {
        newStrGEP = populateEnd(isNumber, &loopEndBuilder, encodedGlobalVar, newAlloca, 
            zero, loopBound, ConstantInt::get(i32, loopBoundInt/loopIterStep));
    }

    // Moving instruction from I till end in the original block to 
    // for.end (loopEnd) Block 
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
    originalValue->replaceAllUsesWith(newStrGEP);
}

/*___________________________________________________________________________
 *
 * @param User *U, a use of global variable
 * @param Value *val, Value associated with Use *U
 * @return Instruction*, instruction just before U or val (or U itself)
 *___________________________________________________________________________*/
Instruction* getInstructionForValue(User *U, Value *val) {
    Instruction *I = dyn_cast<Instruction>(U);
    if(!I) {
        // The value was an operand in an instruction
        // Hence getting the first user of value, which is the
        // instruction before which we should decode 
        for(User *uu: val->users()) {
            // users are in reverse, hence last in the iteration
            I = dyn_cast<Instruction>(uu);
        }
    }
    // else The value itself is the instruction
    return I;
}

/*___________________________________________________________________________
 *
 * @param User *U, a use of global variable
 * @param Value *val, Value associated with Use *U
 * @param bool *del, this is set to false if U is not an Instruction, else true 
 * @return Instruction*, instruction just before U or val (or U itself)
 *___________________________________________________________________________*/
Instruction* getInstructionForValue(User *U, Value *val, bool *del) {
    Instruction *I = dyn_cast<Instruction>(U);
    if(!I) {
        *del = false;
        // The value was an operand in an instruction
        // Hence getting the first user of value, which is the
        // instruction before which we should decode 
        for(User *uu: val->users()) {
            // users are in reverse, hence last in the iteration
            I = dyn_cast<Instruction>(uu);
        }
    } else {
        // The value itself is the instruction
        *del = true;
    }
    return I;
}

} /* namespace */

void CaesarCipher::decode(GlobalVariable* globalVar, int stringLength, int offset) {
    for(User *U: globalVar->users()) {
        if(Value *val = dyn_cast<Value>(U)) {
            Instruction *I = getInstructionForValue(U, val);
            if(I) {
                // Constant is used, hence decode it
                // else skip decoding
                inlineDecode(true, false, globalVar, stringLength, 1, val, I, offset, populateBodyCaesar);
            }
        }
    }
}

void BitEncodingAndDecoding::decode(GlobalVariable* globalVar, GlobalVariable *newStringGlobalVar, int stringLength, int nBits) {
    std::vector<Instruction*> toErase;
    bool del;
    for(User *U: globalVar->users()) {
        if(Value *val = dyn_cast<Value>(U)) {
            Instruction *I = getInstructionForValue(U, val, &del);
            if(I) {
                // Constant is used, hence decode it
                // else skip decoding
                inlineDecode(false, false, globalVar, stringLength, (8/nBits), val, I, nBits, 
                    populateBodyBitEncodingAndDecoding, newStringGlobalVar);
                if(del) {
                    toErase.push_back(I);
                }
            }
        }
    }
    for(Instruction *I : toErase) {
        I->eraseFromParent();
    }
}

void BitEncodingAndDecoding::decodeNumber(GlobalVariable* globalVar, Value *val, Instruction *I, 
    int integerBits, int nBits, LLVMContext& context) {
    inlineDecode(false, true, globalVar, (integerBits/nBits), 1, val, I, nBits,
                    populateBodyBitEncodingAndDecodingNumbers, nullptr, integerBits, &context);
}