#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/GlobalVariable.h"
#include "ConstantsEncoding.h"
using namespace llvm;

#define DEBUG_TYPE "const-encoding"

template<typename T>
void ReplaceUnsafe(T *from, T *to) {
	for(Value::use_iterator it = from->use_begin(); it!=from->use_end(); it++ ) {
    	auto *U = &*it;
    	U->set(to);
	}
  from->eraseFromParent();
}

bool ConstantsEncoding::runOnModule(Module &M) {
	int stringLength;
    static LLVMContext& context = M.getContext();

  	GlobalVariable *globalVar;
  	std::vector<GlobalVariable*> gvs;
	for(Module::global_iterator it = M.global_begin(); it!=M.global_end(); it++) {
		// TODO: check if its string or any other constant
		gvs.push_back(&*it);
	}
  	std::string svalue = "this is a string";
  	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),svalue.size()+1);
	Constant *aString = ConstantDataArray::getString(context, "this is a string", true);
  	GlobalVariable *GV = new GlobalVariable( M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	GV->setAlignment(1);

	for(GlobalVariable *globalVar : gvs) {
		if(globalVar->isConstant() && globalVar->hasInitializer()) {
			// globalVar->replaceAllUsesWith(GV);
			// ReplaceUnsafe(globalVar,GV);
			int offset = BitEncodingAndDecoding::encode(globalVar, &stringLength, &M);
			// CaesarCipher::decode(&*it, stringLength, offset);
		}
	}

    return true;
}

// Registering the pass
char ConstantsEncoding::ID = 0;
static RegisterPass<ConstantsEncoding> X("const-encoding", "Obfuscates string constants");

#undef DEBUG_TYPE
