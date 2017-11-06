#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/GlobalVariable.h"
#include "ConstantsEncoding.h"
using namespace llvm;

#define DEBUG_TYPE "const-encoding"

bool ConstantsEncoding::runOnModule(Module &M) {
    // For bit encoding and decoding new global variable will be 
    // created. Hence string the original global variables in a 
    // vector and iterating over it.
	std::vector<GlobalVariable*> gvs;
	for(Module::global_iterator it = M.global_begin(); it!=M.global_end(); it++) {
		// TODO: check if its string or any other constant
		gvs.push_back(&*it);
	}
	int stringLength;
	for(GlobalVariable *globalVar : gvs) {
		if(globalVar->isConstant() && globalVar->hasInitializer()) {
			// Caesar
			if(rand()%2) {
				int offset = CaesarCipher::encode(globalVar, &stringLength);
				CaesarCipher::decode(globalVar, stringLength, offset);
			} else {
				// Bit encoding and decoding
				GlobalVariable *newStringGlobalVar = nullptr;
				int nBits = BitEncodingAndDecoding::encode(globalVar, &newStringGlobalVar, &stringLength, &M);
				BitEncodingAndDecoding::decode(globalVar, newStringGlobalVar, stringLength, nBits);
				globalVar->eraseFromParent();
			}

		}
	}

    return true;
}

// Registering the pass
char ConstantsEncoding::ID = 0;
static RegisterPass<ConstantsEncoding> X("const-encoding", "Obfuscates string constants");

#undef DEBUG_TYPE
