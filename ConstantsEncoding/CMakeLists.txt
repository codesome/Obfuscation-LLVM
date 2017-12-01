include_directories(${LLVM_MAIN_SRC_DIR}/include/llvm/Transforms/Obfuscation)

add_llvm_loadable_module(ConstantsEncoding
	Main.cpp
	Encode.cpp
	Decode.cpp
)
