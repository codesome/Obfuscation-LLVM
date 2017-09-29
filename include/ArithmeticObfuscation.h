#ifndef __ARITHMETIC_OBFUSCATION_H__
#define __ARITHMETIC_OBFUSCATION_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

/* Implemented in ArithmeticObfuscation/Add.cpp */
class AddObfuscator {

private:
	/* declare multiple logics here */

public:
	/*
	 * Obfuscates add operations
	 * NOTE: This function selects random logics from the ones implemented above and applies on IR
	 *
	 * @param Function* to obfuscate
	 * @return true if IR is modified, false otherwise
	 **/
	static bool obfuscate(Function *F);

};

/* Implemented in ArithmeticObfuscation/Sub.cpp */
class SubObfuscator {

private:
	/* declare multiple logics here */

public:
	/*
	 * Obfuscates sub operations
	 * NOTE: This function selects random logics from the ones implemented above and applies on IR
	 *
	 * @param Function* to obfuscate
	 * @return true if IR is modified, false otherwise
	 **/
	static bool obfuscate(Function *F);

};

/* Implemented in ArithmeticObfuscation/Mul.cpp */
class MulObfuscator {

private:
	/* declare multiple logics here */

public:
	/*
	 * Obfuscates mul operations
	 * NOTE: This function selects random logics from the ones implemented above and applies on IR
	 *
	 * @param Function* to obfuscate
	 * @return true if IR is modified, false otherwise
	 **/
	static bool obfuscate(Function *F);

};

/* Implemented in ArithmeticObfuscation/Div.cpp */
class DivObfuscator {

private:
	/* declare multiple logics here */

public:
	/*
	 * Obfuscates div operations
	 * NOTE: This function selects random logics from the ones implemented above and applies on IR
	 *
	 * @param Function* to obfuscate
	 * @return true if IR is modified, false otherwise
	 **/
	static bool obfuscate(Function *F);

};


class ArithmeticObfuscation : public FunctionPass {

public:
    static char ID;

    ArithmeticObfuscation() : FunctionPass(ID) {}

    bool runOnFunction(Function &F);
    
    void getAnalysisUsage(AnalysisUsage &AU) const override;

};

#endif
