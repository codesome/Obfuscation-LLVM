#ifndef __ARITHMETIC_OBFUSCATION_H__
#define __ARITHMETIC_OBFUSCATION_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
using namespace llvm;

/*
    Structure of AddObfuscator, SubObfuscator, 
        MulObfuscator, SDivObfuscator, UDivObfuscator

     *_____________________________________________________
     *
     * Obfuscates given instruction
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *_____________________________________________________
    static bool obfuscate(Instruction *I);
*/

/* Implemented in ArithmeticObfuscation/Add.cpp */
namespace AddObfuscator {
    bool obfuscate(Instruction *I);
}

/* Implemented in ArithmeticObfuscation/Sub.cpp */
namespace SubObfuscator {
    bool obfuscate(Instruction *I);
}

/* Implemented in ArithmeticObfuscation/Mul.cpp */
namespace MulObfuscator {
    bool obfuscate(Instruction *I);
}

/* Implemented in ArithmeticObfuscation/Div.cpp */
namespace DivObfuscator {
    bool obfuscate(Instruction *I);
}

/* Implemented in ArithmeticObfuscation/ArithmeticObfuscationUtils.cpp */
namespace ArithmeticObfuscationUtils {

/*____________________________________________________
 *
 * Obfuscates binary float operation (a op b)
 * As there is only 2 line difference in fadd,fsub obfuscation 
 * implementation, both are clubbed into same function
 * NOTE: should be used only for fadd and fsub
 *
 * @param Instruction *I, the instruction to obfuscate
 * @param double maxAllowedValue, the max allowed value for operands
 * @param {
 *        Value* (*ifThenCaller)(IRBuilder<>* ifThenBuilder, 
 *              Type* floatType, Value* aXX, Value* bXX, 
 *              Value* aYY, Value* bYY, Value* aXXFloat, Value* bXXFloat),
 *        This is a function called to build the if.then block
 *        @param:
 *            int64: aXX = int64(a), bXX = int64(b)
 *            floatType: aXXFloat = floatType(aXX), bXXFloat = floatType(bXX), 
 *                       aYY = a - aXXFloat, bYY = b - bXXFloat
 *        @return Value*, the result from if.then
 * }
 * @param { 
 *        Value* (*ifElseCaller)(IRBuilder<>* ifElseBuilder, Value* a, Value* b),
 *        This is a function called to build the if.else block
 *        @param
 *            floatType: a, b
 *        @return Value*, the result from if.else
 * }
 * NOTE: Do not create break statements in ifThenCaller and ifElseCaller
 *____________________________________________________*/
void floatObfuscator(
    Instruction *I,
    double maxAllowedValue,
    Value* (*ifThenCaller)(IRBuilder<>*, Type*, Value*, Value*, Value*, Value*, Value*, Value*), 
    Value* (*ifElseCaller)(IRBuilder<>*, Value*, Value*));

} /* namespace ArithmeticObfuscationUtils */

class ArithmeticObfuscation : public FunctionPass {

public:
    static char ID;

    ArithmeticObfuscation() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);

    /*____________________________________________________
     *
     * Obfuscates given basic block
     * @param BasicBlock *BB, the basic block to obfuscate
     * @param bool obfuscateFloat, true if floating point 
        operation has to be obfuscated, false otherwise
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(BasicBlock *BB, bool obfuscateFloat);

    /*____________________________________________________
     *
     * Obfuscates given instruction
     * Does not obfuscate floating point binary oprations
     * NOTE: Does not remove or erase the instruction 
     *       from the block, but changes all the uses. 
     *       Need to erase it manually.
     * @param Instruction *I, the instruction to obfuscate
     * @return true if IR is modified, false otherwise
     *____________________________________________________*/
    static bool obfuscate(Instruction *I);
    
    // Same as 'obfuscate(Instruction *I)' with floats enabled
    static bool obfuscateWithFloat(Instruction *I);

};

#endif
