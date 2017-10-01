#ifndef __ARITHMETIC_OBFUSCATION_H__
#define __ARITHMETIC_OBFUSCATION_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

/* Implemented in ArithmeticObfuscation/Add.cpp */
/* NOTE: Obfuscates only add operation */
class AddObfuscator {
public:
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Function *F);
    
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(BasicBlock *BB);

    /*
     * NOTE: Does not remove or erase the instruction from the block
     *       But changes all the uses. Need to erase it manually.
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Instruction *I);
};

/* Implemented in ArithmeticObfuscation/Sub.cpp */
/* NOTE: Obfuscates only sub operation */
class SubObfuscator {

public:
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Function *F);
    
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(BasicBlock *BB);

    /*
     * NOTE: Does not remove or erase the instruction from the block
     *       But changes all the uses. Need to erase it manually.
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Instruction *I);

};

/* Implemented in ArithmeticObfuscation/Mul.cpp */
/* NOTE: Obfuscates only mul operation */
class MulObfuscator {

public:
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Function *F);
    
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(BasicBlock *BB);

    /*
     * NOTE: Does not remove or erase the instruction from the block
     *       But changes all the uses. Need to erase it manually.
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Instruction *I);

};

/* Implemented in ArithmeticObfuscation/Div.cpp */
/* NOTE: Obfuscates only div operation */
class DivObfuscator {

public:
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Function *F);
    
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(BasicBlock *BB);

    /*
     * NOTE: Does not remove or erase the instruction from the block
     *       But changes all the uses. Need to erase it manually.
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Instruction *I);

};

class SDivObfuscator {

public:
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Function *F);
    
    /*
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(BasicBlock *BB);

    /*
     * NOTE: Does not remove or erase the instruction from the block
     *       But changes all the uses. Need to erase it manually.
     * @return true if IR is modified, false otherwise
     **/
    static bool obfuscate(Instruction *I);

};

class ArithmeticObfuscation : public FunctionPass {

public:
    static char ID;

    ArithmeticObfuscation() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);
    
    void getAnalysisUsage(AnalysisUsage &AU) const override;

};

#endif
