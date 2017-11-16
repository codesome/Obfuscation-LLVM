#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ConstantsEncoding.h"
using namespace llvm;


int CaesarCipher::encode(GlobalVariable* globalVar, int *stringLength){

	// Converting global variable to constant
	Constant* constValue = globalVar->getInitializer();
	// Type casting from constant to constant data array
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	
	if(!result->isCString())
		return CaesarCipher::INVALID;

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



int BitEncodingAndDecoding::encode(GlobalVariable* globalVar,GlobalVariable **newStringGlobalVar, int *stringLength, Module *M){

	// Converting global variable to constant
	Constant* constValue = globalVar->getInitializer();
	// Type casting from constant to constant data array
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);

	if(!result->isCString())
		return BitEncodingAndDecoding::INVALID;
	
	// Storing the value of string in str
	std::string str = result->getAsCString();

	// len = length of string
	int len = str.length();
	// Number of bits which are obfuscated
	int nBits  = 2;
	/* 
		step = Number of steps taken. 
		Each character is of size 8 bits, which is splitted into sets of size nBits.
		Therefore number of steps = Total number of bits/nBits = 8/nBits
	*/
	int step = 8/nBits;

	/*
		After obfuscation, the length of string is increased. 
		Each character is split into sets of nBits and total number of such sets is equal to number of steps.
		Therefore length of encoded string = length of original string * number of steps
	*/
	*stringLength = len*step;

	// Size of encoded string is equal to stringLength + 1 to store null character in the end
	char *encodedStr = new char[step*len+1];
	encodedStr[step*len] = 0;

	// mask = 2^(nBits)-1
	char mask = 1;
    for(int i=1; i<nBits; i++) {
        mask = (mask<<1) + 1;
    }
  
  	// y = maximum value of char left shifted by nBits
	char y = 0xff << nBits;
	char lastnBits;

	// Iterating for each character in string
	for(int i=0;i<len;i++){

		// For each step, updating nBits in encoded string
		for(int j=0;j<step;j++){

			// Gives the last n bits of original character
			lastnBits = str[i] & mask;

			// Current position in encoded string
			int pos = i*step+j;

			// Generates random number from 1 to 127
			int randomNumber = rand() % 127 + 1;

			// Storing random number in encoded character
			encodedStr[pos] = (char)randomNumber;

			// Gives first n bits of the generated random number
			char firstnBits = encodedStr[pos] & y;

			/* 
				encoded character stores last n bits of original char 
				and remaining (8-n) bits of the random number.
			*/
			char encodedChar = lastnBits | firstnBits;

			// if 0, put 11111100
			encodedStr[pos]= (encodedChar==0)? y: encodedChar; 

			// Updating last two bits
			str[i]=str[i]>>nBits;

		}

	}

	// Creating a new global string which is the encoded string
    static LLVMContext& context = globalVar->getContext();
	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),strlen(encodedStr)+1);
	Constant *aString = ConstantDataArray::getString(context, encodedStr, true);

	// giving new variable to decode
  	*newStringGlobalVar = new GlobalVariable(*M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	(*newStringGlobalVar)->setAlignment(1);

	return nBits;
}

int BitEncodingAndDecoding::encodeNumber(GlobalVariable **globalVar, long num, int integerBits, Module *M) {


	// Number of bits which are obfuscated
	int nBits  = 4;

	int len = integerBits/nBits;
	char *encodedStr = new char[len+1];
	encodedStr[len] = 0;

	char mask = 1;
    for(int i=1; i<nBits; i++) {
        mask = (mask<<1) + 1;
    }
  
  	// y = maximum value of char left shifted by nBits
	char y = 0xff << nBits;
	char lastnBits;

	for(int i=0;i<len;i++) {
		lastnBits = char(num) & mask;

		int randomNumber = rand() % 127 + 1;

		encodedStr[i] = (char)randomNumber;

		char firstnBits = encodedStr[i] & y;

		char encodedChar = lastnBits | firstnBits;

		encodedStr[i]= (encodedChar==0)? y: encodedChar; 

		num=num>>nBits;

	}


	// Creating a new global string which is the encoded string
    static LLVMContext& context = M->getContext();
	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),strlen(encodedStr)+1);
	Constant *aString = ConstantDataArray::getString(context, encodedStr, true);
  	*globalVar = new GlobalVariable(*M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	(*globalVar)->setAlignment(1);

  	return nBits;

}
