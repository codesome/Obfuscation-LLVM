include_directories(../include)

add_llvm_loadable_module(ConstantsEncoding
	Main.cpp
	Encode.cpp
	Decode.cpp
)
