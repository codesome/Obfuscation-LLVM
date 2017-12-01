include_directories(${LLVM_MAIN_SRC_DIR}/include/llvm/Transforms/Obfuscation)

add_llvm_loadable_module(ArithmeticObfuscation
    Add.cpp	
	Sub.cpp	
	Mul.cpp	
	Div.cpp
	ArithmeticObfuscationUtils.cpp
	Main.cpp
)
