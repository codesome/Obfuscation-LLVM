#ifndef __CONSTANTS_ENCODING_H__
#define __CONSTANTS_ENCODING_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/GlobalVariable.h"

using namespace llvm;


class CaesarCipher {
public:
    /*____________________________________________________
     *
     * Encodes the global variable and replaces encoded
     * constant in IR
     * NOTE: Only encode for string is implemented
     *
     * @param GlobalVariabel* globalVar, variable to encode
     * @param int *stringLength, the string length will be stored in this
     * @return int, the offset used to obfuscate
     *____________________________________________________*/
    static int encode(GlobalVariable* globalVar, int *stringLength);

    /*____________________________________________________
     *
     * Adds inline decode function in IR whereever 
     * the constant is used
     * NOTE: Only decode for string is implemented
     *
     * @param GlobalVariabel* globalVar, variable to decode in IR
     * @param int stringLength, the encoded string length
     * @param int offset, the offset used to encode
     *____________________________________________________*/
    static void decode(GlobalVariable* globalVar, int stringLength, int offset);
};

class BitEncodingAndDecoding {
public:
    static int encode(GlobalVariable* globalVar, int *originalStringLength, Module *M);
    static void decode(GlobalVariable* globalVar, int originalStringLength, int nBits);
};

class ConstantsEncoding : public ModulePass {

public:
    static char ID;

    ConstantsEncoding() : ModulePass(ID) {}

    bool runOnModule(Module &M);

};

#endif
