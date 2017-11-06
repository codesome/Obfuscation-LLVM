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
    /*___________________________________________________________________
     *
     * Encodes the global variable and replaces encoded
     * constant in IR
     * NOTE: Only encode for string is implemented
     *
     * @param GlobalVariabel* globalVar, variable to encode
     * @param int *stringLength, the string length will be stored in this
     * @return int, the offset used to obfuscate
     *___________________________________________________________________*/
    static int encode(GlobalVariable* globalVar, int *stringLength);

    /*___________________________________________________________________
     *
     * Adds inline decode function for Caesar cipher in IR where ever 
     * the constant is used
     * NOTE: Only decode for string is implemented
     *
     * @param GlobalVariabel* globalVar, variable to decode in IR
     * @param int stringLength, the encoded string length
     * @param int offset, the offset used to encode
     *___________________________________________________________________*/
    static void decode(GlobalVariable* globalVar, int stringLength, int offset);
};

class BitEncodingAndDecoding {
public:
    /*___________________________________________________________________
     *
     * Encodes the global variable and stores the encoded
     * string in a new global variable
     * NOTE: Only encode for string is implemented
     *
     * @param GlobalVariabel* globalVar, variable to encode
     * @param GlobalVariabel** newStringGlobalVar, GlobalVariable* 
     *                of the new  global variable is stored in this 
     * @param int *stringLength, the string length of encoded string 
     *                will be stored in this
     * @return int, number of bits encoded in each character
     *___________________________________________________________________*/
    static int encode(GlobalVariable *globalVar, GlobalVariable **newStringGlobalVar, int *stringLength, Module *M);
    
    /*___________________________________________________________________
     *
     * Adds inline decode function for bit-encoding in IR where ever 
     * the constant is used
     * NOTE: Only decode for string is implemented
     *
     * @param GlobalVariabel* globalVar, original variable to remove
     * @param GlobalVariabel** newStringGlobalVar, the encoded variable 
     * @param int stringLength, the encoded string length
     * @param int nBits, number of bits encoded in each character
     *___________________________________________________________________*/
    static void decode(GlobalVariable *globalVar, GlobalVariable *newStringGlobalVar, int stringLength, int nBits);
};

class ConstantsEncoding : public ModulePass {

public:
    static char ID;

    ConstantsEncoding() : ModulePass(ID) {}

    bool runOnModule(Module &M);

};

#endif
