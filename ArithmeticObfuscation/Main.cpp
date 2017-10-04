#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ArithmeticObfuscation.h"
using namespace llvm;

#define DEBUG_TYPE "arith-obfus"

cl::opt<int> numIterations("arith-obfus-iter", cl::desc("<number of iterations (>0 and <10) >"), cl::init(1));

namespace {

bool obfuscate(Instruction *I) {
    switch(I->getOpcode()) {
        case (Instruction::Add):
            return AddObfuscator::obfuscate(I);
        case (Instruction::Sub):
            return SubObfuscator::obfuscate(I);
        case (Instruction::SDiv):
            return SDivObfuscator::obfuscate(I);
        case (Instruction::UDiv):
            return UDivObfuscator::obfuscate(I);
        case (Instruction::Mul):
            return MulObfuscator::obfuscate(I);
        default:
            return false;
    }
}

} /* namespace */

bool ArithmeticObfuscation::runOnFunction(Function &F) {

    int nIter = numIterations;

    if(nIter <0) {
        nIter = 1;
        DEBUG(dbgs() << "Number of iterations given is <0. Setting it to 1.\n");
    }
    else if(nIter > 10) {
        DEBUG(dbgs() << "Number of iterations given is >10. Setting it to 10.\n");
        nIter = 10;
    }

    bool modified = false;
    for(int i=0; i<nIter; i++) {
        bool iterModified = false;

        // original instruction which got obfuscated
        std::vector<Instruction *> toErase;
        std::vector<BasicBlock*> toIterate;
        for(BasicBlock &BB : F) {
          toIterate.push_back(&BB);
        }
        // Consider only existing basic blocks
        // Ignore basic blocks which have been created due to obfuscate call
        for(BasicBlock *BB : toIterate) {
            for(Instruction &I : *BB) {
                if(obfuscate(&I)) {
                    iterModified = true;
                    toErase.push_back(&I);
                }
            }
        }

        // Instructions are not removed or erased from IR
        // Erasing all the obfuscated original instructions
        for(Instruction *I: toErase) {
            I->eraseFromParent();
        }
        if(iterModified) {
            modified = true;
        } else {
            break;
        }
    }
    return modified;
}

void ArithmeticObfuscation::getAnalysisUsage(AnalysisUsage &AU) const {

}


// Registering the pass
char ArithmeticObfuscation::ID = 0;
static RegisterPass<ArithmeticObfuscation> X("arith-obfus", "Obfuscates arithmetic operations");

#undef DEBUG_TYPE
