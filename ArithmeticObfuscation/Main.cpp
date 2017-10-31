#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ArithmeticObfuscation.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

#define DEBUG_TYPE "arith-obfus"

cl::opt<int> numIterations("arith-obfus-iter", cl::desc("<number of iterations (>0 and <=3) >"), cl::init(1));
cl::opt<bool> obfuscateFloat("obfus-float", cl::desc("Enable obfuscation of floating point binary operations"), cl::init(false));

bool ArithmeticObfuscation::obfuscate(Instruction *I) {
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

bool ArithmeticObfuscation::obfuscateWithFloat(Instruction *I) {
    switch(I->getOpcode()) {
        case (Instruction::Add):
        case (Instruction::FAdd):
            return AddObfuscator::obfuscate(I);
        case (Instruction::Sub):
        case (Instruction::FSub):
            return SubObfuscator::obfuscate(I);
        case (Instruction::SDiv):
            return SDivObfuscator::obfuscate(I);
        case (Instruction::UDiv):
            return UDivObfuscator::obfuscate(I);
        case (Instruction::Mul):
        case (Instruction::FMul):
            return MulObfuscator::obfuscate(I);
        default:
            return false;
    }
}

bool ArithmeticObfuscation::obfuscate(BasicBlock *BB, bool oFloat) {
    bool modified = false;
    std::vector<Instruction *> toIterateInst;
    std::vector<Instruction *> toErase;
    // Instructions after this will get moved from the block for
    // fadd. Hence first storing all the instructions.
    for(Instruction &I : *BB) {
        toIterateInst.push_back(&I);
    }
    if(oFloat) {
        for(Instruction *I : toIterateInst) {
            if(obfuscateWithFloat(I)) {
                modified = true;
                toErase.push_back(I);
            }
        }
    } else {
        for(Instruction *I : toIterateInst) {
            if(obfuscate(I)) {
                modified = true;
                toErase.push_back(I);
            }
        }
    }
    for(Instruction *I: toErase) {
        I->eraseFromParent();
    }
    return modified;
}

bool ArithmeticObfuscation::runOnFunction(Function &F) {

    int nIter = numIterations;
    bool obfusFloat = obfuscateFloat;
    if(nIter <= 0) {
        // should have atleast 1
        nIter = 1;
        DEBUG(dbgs() << "Number of iterations given is <=0. Setting it to 1.\n");
    } else if(nIter > 3) {
        // Not allowing more than 3, this is for efficiency of code
        // The code size grows exponentially with nIter for floats
        DEBUG(dbgs() << "Number of iterations given is >3. Setting it to 3.\n");
        nIter = 3;
    }

    bool modified = false;
    for(int i=0; i<nIter; i++) {
        bool iterModified = false;
        // original instruction which got obfuscated
        std::vector<BasicBlock*> toIterate;
        for(BasicBlock &BB : F) {
          toIterate.push_back(&BB);
        }
        // Consider only existing basic blocks
        // Ignore basic blocks which have been created due to obfuscate call
        for(BasicBlock *BB : toIterate) {
            iterModified = obfuscate(BB, obfusFloat) || iterModified; 
        }
        if(iterModified) {
            modified = true;
        } else {
            // if nothing was modified in this iteration
            // then no use of going to next iteration
            break;
        }
    }
    return modified;
}

// Registering the pass
char ArithmeticObfuscation::ID = 0;
static RegisterPass<ArithmeticObfuscation> X("arith-obfus", "Obfuscates arithmetic operations");

#undef DEBUG_TYPE
