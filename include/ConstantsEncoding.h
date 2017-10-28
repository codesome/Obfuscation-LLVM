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


class ConstantsEncodingUtils {
public:
	static int encode(GlobalVariable*);

	static void decode(GlobalVariable*, int);
};

class ConstantsEncoding : public ModulePass {

public:
    static char ID;

    ConstantsEncoding() : ModulePass(ID) {}

    bool runOnModule(Module &M);

};

#endif
