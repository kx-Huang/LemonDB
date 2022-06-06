# Compilation

## Prerequisites

1. C++ compiler: Clang >= 6.0
2. CMake >= 2.7

## Linux/macOS Quick Start

```bash
$ mkdir build && cd build
$ cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ../src
$ cmake --build . -- -j64
$ cd ..
```

If everything goes fine, you will have a lemondb binary `./build/lemondb`.
