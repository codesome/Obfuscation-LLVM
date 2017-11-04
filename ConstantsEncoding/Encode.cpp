#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ConstantsEncoding.h"
using namespace llvm;
#define DEBUG_TYPE "const-encoding"

int CaesarCipher::encode(GlobalVariable* globalVar, int *stringLength){

	// Converting global variable to constant
	Constant* constValue = globalVar->getInitializer();
	// Type casting from constant to constant data array
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	// Storing the value of string in str
	std::string str = result->getAsCString();
	// Getting random number
	int randomNumber = rand() % 125 + 1;

	// Encoding string by adding the random number in each character
	int len = str.length();
	*stringLength = len;
	for(int i=0;i<len;i++){
		str[i] = 1+(str[i]+randomNumber)%127;
	}

	Constant *encodedStr = ConstantDataArray::getString(globalVar->getContext(), str, true);
	// Modifing global variable in IR
	globalVar->setInitializer(encodedStr);
	
	// returning the value of random number
	return randomNumber;
}



int BitEncodingAndDecoding::encode(GlobalVariable* globalVar, int *stringLength, Module *M){

	// Converting global variable to constant
	Constant* constValue = globalVar->getInitializer();
	// Type casting from constant to constant data array
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	// Storing the value of string in str
	std::string str = result->getAsCString();

	// Encoding string by adding the random number in each character
	int len = str.length();
	dbgs() << len << " :: len\n";
	int n  = 2;
	int step = 8/n;
	*stringLength = len*step;
	char *s = new char[step*len];
	char mask = 1;
    for(int i=1; i<n; i++) {
        mask = (mask<<1) + 1;
    }
	char y = 0xff << n;
	char p;
	for(int i=0;i<len;i++){

		for(int j=0;j<step;j++){
			p = str[i] & mask;
			int pos = i*step+j;
			int randomNumber = rand() % 127 + 1;

			s[pos] = (char)randomNumber;
			char q = s[pos] & y;
			char fv = p | q;
			s[pos]=fv;
			str[i]=str[i]>>n;

		}

	}

	dbgs()<<s<<"\n";
    static LLVMContext& context = globalVar->getContext();
	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),(len*step)+1+1);
	Constant *aString = ConstantDataArray::getString(context, s, false);
	dbgs() << strlen(s) << "\n";
	dbgs() << len*step + 1 << "\n";
	dbgs() << *(aString->getType()) << "\n";
  	GlobalVariable *GV = new GlobalVariable( *M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	GV->setAlignment(1);


	// Constant *encodedStr = ConstantDataArray::getString(globalVar->getContext(), s, true);
	// // Modifing global variable in IR
	// globalVar->setInitializer(encodedStr);

	return 0;
}