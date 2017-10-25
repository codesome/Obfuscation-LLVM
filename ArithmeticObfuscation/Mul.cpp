#include "ArithmeticObfuscation.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

bool MulObfuscator::obfuscate(Instruction *I) {
	if(I->getOpcode() != Instruction::Mul)
			return false;
	Type* type = I->getType();
	if(!type->isIntegerTy())
			return false;
	bool isMultiplicandLoad = false;
	Value* vmultiplicand = I->getOperand(0);
	if(dyn_cast<LoadInst>(vmultiplicand)){
		//Get definition of multiplicand if vmultiplicand is a load instruction
		vmultiplicand = (dyn_cast<LoadInst>(vmultiplicand)->getOperand(0));
		isMultiplicandLoad = true;
	}
	Value* vmultiplierLoad = I->getOperand(1);
	bool isMultiplierLoad = false;
	Value* vmultiplier = vmultiplierLoad;
	if(dyn_cast<LoadInst>(vmultiplier)){
		//Get definition of multiplicand if vmultiplicand is a load instruction
		vmultiplier = (dyn_cast<LoadInst>(vmultiplier)->getOperand(0));
		isMultiplierLoad = true;
	}
	static LLVMContext& Context = I->getParent()->getContext();

	IRBuilder<> Builder(I);
	//i=1; j=0; temp=multiplier;
	auto* allocaj = Builder.CreateAlloca(type);
	auto* allocai = Builder.CreateAlloca(type);
	auto* allocaTemp = Builder.CreateAlloca(type);
	auto* allocak = Builder.CreateAlloca(type);
	Builder.CreateStore(ConstantInt::get(type,0),allocaj);
	Builder.CreateStore(ConstantInt::get(type,1),allocai);
	Builder.CreateStore(vmultiplierLoad,allocaTemp);

	//Loop header
	BasicBlock* bbHeader = BasicBlock::Create(Context,"",I->getParent()->getParent());
	Builder.CreateBr(bbHeader);

	//Find number of times multiplicand should be shifted and number of times it should be added
	//j denotes number of times multiplicand needs to be shifted
	//i denotes number of times multiplicand should be added

	//loop if temp>1
	IRBuilder<> BuilderHeader(bbHeader);
	auto* loadMultiplier = BuilderHeader.CreateLoad(allocaTemp);
	auto* icmpgt1 = BuilderHeader.CreateICmpSGT(loadMultiplier,ConstantInt::get(type,1));
	auto* bbTrue = BasicBlock::Create(Context,"true",I->getParent()->getParent());
	auto* bbFalse = BasicBlock::Create(Context,"false",I->getParent()->getParent());
	BuilderHeader.CreateCondBr(icmpgt1,bbTrue,bbFalse);
	//True block
	//temp = temp>>1
	IRBuilder<> BuilderTrue(bbTrue);
	auto* loadTemp1 = BuilderTrue.CreateLoad(allocaTemp);
	auto* shiftTemp = BuilderTrue.CreateAShr(loadTemp1,ConstantInt::get(type,1));
	BuilderTrue.CreateStore(shiftTemp,allocaTemp);
	//j = j+1
	auto* loadj1 = BuilderTrue.CreateLoad(allocaj);
	auto* addj = BuilderTrue.CreateAdd(loadj1,ConstantInt::get(type,1));
	BuilderTrue.CreateStore(addj,allocaj);
	//i = i<<1
	auto* loadi1 = BuilderTrue.CreateLoad(allocai);
	auto* shifti = BuilderTrue.CreateShl(loadi1,ConstantInt::get(type,1));
	BuilderTrue.CreateStore(shifti,allocai);
	BuilderTrue.CreateBr(bbHeader);

	//False block / exit block
	//i = multiplier - i
	IRBuilder<> BuilderFalse(bbFalse);
	auto* loadi2 = BuilderFalse.CreateLoad(allocai);
	auto* loadMultiplier1 = vmultiplier;
	//If vmultiplier is not a load instruction, create a load instruction to load vmultiplier
	if(isMultiplierLoad)
		loadMultiplier1 = BuilderFalse.CreateLoad(vmultiplier);
	auto* subi = BuilderFalse.CreateSub(loadMultiplier1,loadi2);
	BuilderFalse.CreateStore(subi,allocai);
	//Instead of adding multiplicand i times, multiply i by multiplicand
	//Mul instruction generated would serve as a seed for next iteration of obfuscation
	//k = i*multiplicand
	auto* loadi3 = BuilderFalse.CreateLoad(allocai);
	auto* loadMultiplicand = vmultiplicand;
	//If vmultiplicand is not a load instruction, create a load instruction to load vmultiplicand
	if(isMultiplicandLoad)
	 	loadMultiplicand = BuilderFalse.CreateLoad(vmultiplicand);
	auto* mulk = BuilderFalse.CreateMul(loadi3,loadMultiplicand);
	BuilderFalse.CreateStore(mulk,allocak);
	auto* loadMultiplicand1 = vmultiplicand;
	//If vmultiplicand is not a load instruction, create a load instruction to load vmultiplicand
	if(isMultiplicandLoad)
		loadMultiplicand1 = BuilderFalse.CreateLoad(vmultiplicand);
	//Final calculation => (multiplicand>>j) + k
	auto* loadj3 = BuilderFalse.CreateLoad(allocaj);
	auto* shiftMultiplicand = BuilderFalse.CreateShl(loadMultiplicand1,loadj3);
	auto* loadk1 = BuilderFalse.CreateLoad(allocak);
	auto* addFinal = BuilderFalse.CreateAdd(shiftMultiplicand,loadk1);

	/*move all the instruction in the parent block which are after the substituted mul instruction
		to the exit block
	*/
	Instruction *i = I->getNextNode();
	Instruction* next = dyn_cast<Instruction>(addFinal);
	std::vector<Instruction *> toMove;
	while(i != nullptr)	{
		toMove.push_back(i);
		i=i->getNextNode();
	}
	for(Instruction *I: toMove) {
			I->removeFromParent();
			I->insertAfter(next);
			next = I;
	}

	I->replaceAllUsesWith(addFinal);
	return true;
}
