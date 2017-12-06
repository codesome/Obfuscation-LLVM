#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "ConstantEncoding/ConstantEncoding.h"
#include <random>
using namespace llvm;

namespace {
// Random number generator, better than rand()
std::random_device rd;
std::mt19937 engine(rd());
std::uniform_int_distribution<int> gen(0,1<<30);
}

int CaesarCipher::encode(GlobalVariable* globalVar, int *stringLength){

	// Getting the string value from the global variable
	Constant* constValue = globalVar->getInitializer();
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	if(!result->isCString()) // We only consider C-strings (which ends with 0)
		return CaesarCipher::INVALID;
	// The string in the global variable
	std::string str = result->getAsCString();

	// Getting random number
	int randomNumber = gen(engine) % 125 + 1;

	// Adding offset to all characters
	int len = str.length();
	*stringLength = len;
	for(int i=0;i<len;i++){
		str[i] = 1+(str[i]+randomNumber)%127;
	}

	// Replacing original string with original string in global variable
	Constant *encodedStr = ConstantDataArray::getString(globalVar->getContext(), str, true);
	globalVar->setInitializer(encodedStr);

	return randomNumber;
}

namespace {
// returns random number among 1,2,4
int getRandomNBits() {
	switch(gen(engine)%3) {
		case 0:
			return 1;
		case 1:
			return 2;
		default:
			return 4;
	}
}
}

int BitEncodingAndDecoding::encode(GlobalVariable* globalVar,GlobalVariable **newStringGlobalVar, int *stringLength, Module *M){

	// Getting the string value from the global variable
	Constant* constValue = globalVar->getInitializer();
	ConstantDataArray* result = cast<ConstantDataArray>(constValue);
	if(!result->isCString()) // We only consider C-strings (which ends with 0)
		return BitEncodingAndDecoding::INVALID;
	// The string in the global variable
	std::string str = result->getAsCString();

	int len = str.length();
	// Number of bits which are embedded in each character
	int nBits  = getRandomNBits();
	// step = Number of steps taken. 
	// Each character is of size 8 bits, which is splitted into sets of size nBits.
	// Therefore number of steps = Total number of bits/nBits = 8/nBits
	int step = 8/nBits;

	// After obfuscation, the length of string is increased. 
	// Each character is split into sets of nBits and total 
	// number of such sets is equal to number of steps.
	// Therefore length of encoded string = length of original string * number of steps
	*stringLength = len*step;

	// Size of encoded string is equal to stringLength+(1 to store null character in the end)
	char *encodedStr = new char[step*len+1];
	encodedStr[step*len] = 0;

	// mask = 00000....1111111 (111.. for nBits times)
	char mask = (1<<nBits)-1;
  	// 1's compliment of mask
	// char y = 0xff << nBits;
	char y = ~mask;

	char lastnBits;

	// Iterating for each character in string
	for(int i=0;i<len;i++){
		// For each step, adding new characters for a character
		for(int j=0;j<step;j++){
			// Gives the last n bits of original character
			lastnBits = str[i] & mask;
			// Current position in encoded string
			int pos = i*step+j;
			// Generates random number from 1 to 127
			int randomNumber = gen(engine) % 127 + 1;

			encodedStr[pos] = (char)randomNumber;
			// Gives first n bits of the generated random number
			char firstnBits = encodedStr[pos] & y;

			// encoded character stores last n bits of original char 
			// and remaining (8-n) bits of the random number.
			char encodedChar = lastnBits | firstnBits;

			// if 0, put 11111100
			encodedStr[pos]= (encodedChar==0)? y: encodedChar; 
			// Moving next n bits to the end in original character
			str[i] = str[i]>>nBits;
		}

	}

	// Creating a new global variable to hold encoded string
    static LLVMContext& context = globalVar->getContext();
	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),strlen(encodedStr)+1);
	Constant *aString = ConstantDataArray::getString(context, encodedStr, true);
  	*newStringGlobalVar = new GlobalVariable(*M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	(*newStringGlobalVar)->setAlignment(1);

	return nBits;
}

int BitEncodingAndDecoding::encodeNumber(GlobalVariable **globalVar, long num, int integerBits, Module *M) {

	// Number of bits which are embedded in each character
	int nBits = getRandomNBits();

	int len = integerBits/nBits;
	char *encodedStr = new char[len+1];
	encodedStr[len] = 0;

	// mask = 00000....1111111 (111.. for nBits times)
	char mask = (1<<nBits)-1;
  	// 1's compliment of mask
	char y = ~mask;

	char lastnBits;
	for(int i=0;i<len;i++) {
		// Logic is same as `BitEncodingAndDecoding::encode`
		lastnBits = char(num) & mask;
		int randomNumber = gen(engine) % 127 + 1;
		encodedStr[i] = (char)randomNumber;
		char firstnBits = encodedStr[i] & y;
		char encodedChar = lastnBits | firstnBits;
		encodedStr[i]= (encodedChar==0)? y: encodedChar; 
		num=num>>nBits;
	}

	// Creating a new global variable to hold encoded string
    static LLVMContext& context = M->getContext();
	ArrayType *Ty = ArrayType::get(Type::getInt8Ty(context),strlen(encodedStr)+1);
	Constant *aString = ConstantDataArray::getString(context, encodedStr, true);
  	*globalVar = new GlobalVariable(*M, Ty, true, GlobalValue::PrivateLinkage, aString);
  	(*globalVar)->setAlignment(1);

  	return nBits;

}
