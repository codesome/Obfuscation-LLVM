#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/GlobalVariable.h"
#include "ConstantEncoding/ConstantEncoding.h"
using namespace llvm;

#define DEBUG_TYPE "const-encoding"

bool ConstantEncoding::runOnModule(Module &M) {
	// TODO: storing all instructions is a bad idea. Make it efficient.
	//       Probable store all blocks. And store instructions only for each block.
	ConstantInt *CI;
	std::vector<Instruction*> insToItertate;
	for (Function &F : M) {
		for(BasicBlock &BB: F) {
			for(Instruction &I: BB) {
				insToItertate.push_back(&I);
			}
		}
	}

    // For bit encoding and decoding new global variable will be 
    // created. Hence storing the original global variables in a 
    // vector and iterating over it.
	std::vector<GlobalVariable*> gvs;
	for(Module::global_iterator it = M.global_begin(); it!=M.global_end(); it++) {
		gvs.push_back(&*it);
	}

	// iterating through all operands in all instructions to 
	// encode and decode integers
	for(Instruction *I: insToItertate) {
		if(I->getType()->isIntegerTy()) {
			int numOperands = I->getNumOperands();
			for (int i=0; i < numOperands; i++) {
				if((CI=dyn_cast<ConstantInt>(I->getOperand(i)))!=nullptr) {
					GlobalVariable *globalVar;
					int integerBits = CI->getType()->getIntegerBitWidth();
					long val = CI->getSExtValue();
					int nBits = BitEncodingAndDecoding::encodeNumber(&globalVar, val, integerBits, &M);
					BitEncodingAndDecoding::decodeNumber(globalVar, CI, I, integerBits, nBits, M.getContext());
				}
			}
		}
	}

	int stringLength;
	for(GlobalVariable *globalVar : gvs) {
		if(globalVar->isConstant() && globalVar->hasInitializer()) {
			if(rand()%2) {
				// Caesar
				int offset = CaesarCipher::encode(globalVar, &stringLength);
				if(offset != CaesarCipher::INVALID)
					CaesarCipher::decode(globalVar, stringLength, offset);
			} else {
				// Bit encoding and decoding
				GlobalVariable *newStringGlobalVar = nullptr;
				int nBits = BitEncodingAndDecoding::encode(globalVar, &newStringGlobalVar, &stringLength, &M);
				if(nBits != BitEncodingAndDecoding::INVALID) {
					BitEncodingAndDecoding::decode(globalVar, newStringGlobalVar, stringLength, nBits);
					globalVar->eraseFromParent();
				}
			}

		}
	}


    return true;
}

// Registering the pass
char ConstantEncoding::ID = 0;
static RegisterPass<ConstantEncoding> X("const-encoding", "Obfuscates string constants");

#undef DEBUG_TYPE
