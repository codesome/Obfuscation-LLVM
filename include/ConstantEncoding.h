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


namespace CaesarCipher {

// Used to mark an invalid constant for encoding
const int INVALID = -1;

/*___________________________________________________________________
 *
 * Encodes the global variable and replaces encoded
 * constant in IR
 * NOTE: Only for string
 *
 * @param GlobalVariabel* globalVar, variable to encode
 * @param int *stringLength, the string length will be stored in this
 * @return int, the offset used to obfuscate
 *              CaesarCipher::INVALID if not encoded
 *___________________________________________________________________*/
int encode(GlobalVariable* globalVar, int *stringLength);

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
void decode(GlobalVariable* globalVar, int stringLength, int offset);

} /* namespace CaesarCipher */

namespace BitEncodingAndDecoding {
    
// Used to mark an invalid constant for encoding
const int INVALID = -1;

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
 *              BitEncodingAndDecoding::INVALID if not encoded
 *___________________________________________________________________*/
int encode(GlobalVariable *globalVar, GlobalVariable **newStringGlobalVar, int *stringLength, Module *M);

int encodeNumber(GlobalVariable **globalVar, long num, int integerBits, Module *M);
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
void decode(GlobalVariable *globalVar, GlobalVariable *newStringGlobalVar, int stringLength, int nBits);

void decodeNumber(GlobalVariable* globalVar, Value *val, Instruction *I, int integerBits, int nBits, LLVMContext& ctx);

} /* namespace BitEncodingAndDecoding */

class ConstantEncoding : public ModulePass {

public:
    static char ID;

    ConstantEncoding() : ModulePass(ID) {}

    bool runOnModule(Module &M);

};

#endif
