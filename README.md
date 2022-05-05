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
