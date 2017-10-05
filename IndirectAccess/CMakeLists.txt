include_directories(../include)

add_llvm_loadable_module(IndirectAccess
	CheckLegality.cpp
	LoopSplit.cpp
	UpdateAccess.cpp
	Main.cpp
)
