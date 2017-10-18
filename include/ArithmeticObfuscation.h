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
    /*____________________________________________________
     *
     * Obfuscates entire function
     * @param Function* F, the function to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Function *F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB);


    /*____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
};

/* Implemented in ArithmeticObfuscation/Sub.cpp */
/* NOTE: Obfuscates only sub operation */
class SubObfuscator {

public:
    /*____________________________________________________
     *
     * Obfuscates entire function
     * @param Function* F, the function to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Function *F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB);


    /*____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
};

/* Implemented in ArithmeticObfuscation/Mul.cpp */
/* NOTE: Obfuscates only mul operation */
class MulObfuscator {

public:
    /*____________________________________________________
     *
     * Obfuscates entire function
     * @param Function* F, the function to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Function *F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB);


    /*____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
};

/* Implemented in ArithmeticObfuscation/UDiv.cpp */
/* NOTE: Obfuscates only udiv operation */
class UDivObfuscator {

public:
    /*____________________________________________________
     *
     * Obfuscates entire function
     * @param Function* F, the function to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Function *F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB);


    /*____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
};

/* Implemented in ArithmeticObfuscation/SDiv.cpp */
/* NOTE: Obfuscates only sdiv operation */
class SDivObfuscator {

public:
    /*____________________________________________________
     *
     * Obfuscates entire function
     * @param Function* F, the function to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Function *F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB);


    /*____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
};

class ArithmeticObfuscation : public FunctionPass {

public:
    static char ID;

    ArithmeticObfuscation() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);
};

#endif
