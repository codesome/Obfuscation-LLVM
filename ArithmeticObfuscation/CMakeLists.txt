include_directories(../include)

add_llvm_loadable_module(ArithmeticObfuscation
    Add.cpp	
	Sub.cpp	
	Mul.cpp	
	UDiv.cpp
	SDiv.cpp	
	ArithmeticObfuscationUtils.cpp
	Main.cpp
)
