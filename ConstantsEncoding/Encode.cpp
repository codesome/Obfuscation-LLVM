#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ConstantsEncoding.h"
using namespace llvm;

int ConstantsEncodingUtils::encode(GlobalVariable* globalVar){

	// Converting global variable to constant
	Constant* constValue = globalVar->getInitializer();
	// Type casting from constant to constant data array
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	// Storing the value of string in str
	std::string str = result->getAsCString();
	// Getting random number
	int randomNumber = rand() % 125 + 1;

	// Encoding string by adding the random number in each character
	unsigned len = str.length();
	for(unsigned i=0;i<len;i++){
		str[i] = 1+(str[i]+randomNumber)%127;
	}

	Constant *encodedStr = ConstantDataArray::getString(globalVar->getContext(), str, true);
	// Modifing global variable in IR
	globalVar->setInitializer(encodedStr);
	
	// returning the value of random number
	return randomNumber;
}