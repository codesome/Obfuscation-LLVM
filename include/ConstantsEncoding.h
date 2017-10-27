#ifndef __CONSTANTS_ENCODING_H__
#define __CONSTANTS_ENCODING_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/LLVMContext.h"
using namespace llvm;


class ConstantsEncodingUtils {
public:
};

class ConstantsEncoding : public FunctionPass {

public:
    static char ID;

    ConstantsEncoding() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);

};

#endif