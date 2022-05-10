# CS526 Project2

### Build
We assume the use of `Ninja`.
First build `boolector`:
```console
git submodule update --init --recursive
cd boolector
./contrib/setup-lingeling.sh
./contrib/setup-btor2tools.sh
./configure.sh --ninja -fPIC && cd build && cmake --build .
```

Then we can build the project
```console
mkdir build && cd build
export LLVM_DIR='/path/to/your/lib/cmake/llvm'
cmake -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS=-pipe -DCMAKE_CXX_FLAGS=-pipe ..
cmake --build .
```

Simple unit test:
```console
cd test/unit
make
```
You should see part of the test results as the following:
```console
Possible Integer error: <stdin>::sadd_overflow:   %1 = add nsw i32 1, 2147483647
Possible Integer error: <stdin>::uadd_overflow:   %1 = add i32 1, -1
Possible Integer error: <stdin>::ssub_overflow:   %1 = sub nsw i32 -2, 2147483647
Possible Integer error: <stdin>::usub_overflow:   %1 = sub i32 1, 2
Possible Integer error: <stdin>::smul_overflow:   %1 = mul nsw i32 2, 1073741824
Possible Integer error: <stdin>::umul_overflow:   %1 = mul i32 2, -2147483648
Possible Integer error: <stdin>::shl_error:   %2 = shl i32 2147483647, 32
Possible Integer error: <stdin>::lshr_error:   %2 = lshr i32 -1, 32
Possible Integer error: <stdin>::ashr_error:   %2 = ashr i32 -1, 32
Possible Integer error: <stdin>::udiv_error:   %2 = udiv i32 2, 0
Possible Integer error: <stdin>::sdiv_error1:   %6 = sdiv i32 2, 0
Possible Integer error: <stdin>::sdiv_error2:   %6 = sdiv i32 -2147483648, -1
```

The corresponding C code is in `test_single_op.c`.
