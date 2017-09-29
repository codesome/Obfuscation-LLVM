# Obfuscation-LLVM

Have a look at `include/ArithmeticObfuscation.h` before starting to code.

#### Using 
```
$ cd $LLVM_DIR/lib/Transforms
$ git clone https://github.com/thecodesome/Obfuscation-LLVM.git Obfuscation

# Add 'add_subdirectory(Obfuscation)' in $LLVM_DIR/lib/Transforms/CMakeLists.txt

$ cd $LLVM_BUILD
# run your cmake command
$ make ArithmeticObfuscation

# Load $LLVM_BUILD/lib/ArithmeticObfuscation.so and use flag '-arith-obfus'
# to use this pass

```
