# Obfuscation-LLVM

### Build
```
$ cd $LLVM_DIR/lib/Transforms
$ git clone https://github.com/thecodesome/Obfuscation-LLVM.git Obfuscation

# Add 'add_subdirectory(Obfuscation)' in $LLVM_DIR/lib/Transforms/CMakeLists.txt

$ cd $LLVM_BUILD
# run your cmake command
$ make -j{NUM_PROCS} ArithmeticObfuscation IndirectAccess

```
### Passes

#### 1. Arithmetic Obfucation `-arith-obfus`

Load `$LLVM_BUILD/lib/ArithmeticObfuscation.so` and use `-arith-obfus` flag.

Additional flag: `-arith-obfus-iter=N`, where `N` an integer, `1 <= N <= 10`. It is the max number of iterations over the IR. Every iteration obfuscates the arithmetic expressions generated in previous iterations. The default value is `N=1`

#### 2. Indirect Access `-indirect-access`

Load `$LLVM_BUILD/lib/IndirectAccess.so` and use `-loop-rotate -indirect-access` flag.

NOTE: `loop-rotate` should be used before `indirect-access`
