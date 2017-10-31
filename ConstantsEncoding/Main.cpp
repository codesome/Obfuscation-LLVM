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
	int stringLength;
	GlobalVariable *globalVar;
	for(Module::global_iterator it = M.global_begin(); it!=M.global_end(); it++) {
		// TODO: check if its string or any other constant
		globalVar = &*it;
		if(globalVar->isConstant() && globalVar->hasInitializer()) {
			int offset = ConstantsEncodingUtils::encode(globalVar, &stringLength);
			ConstantsEncodingUtils::decode(&*it, stringLength, offset);
		}
	}
    return true;
}

// Registering the pass
char ConstantsEncoding::ID = 0;
static RegisterPass<ConstantsEncoding> X("const-encoding", "Obfuscates string constants");

#undef DEBUG_TYPE
