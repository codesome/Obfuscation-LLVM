include_directories(../include)

add_llvm_loadable_module(IndirectAccess
	Main.cpp
)
