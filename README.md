# LLVM PASS (The tool I use to do mutation)

This is a subproject of https://github.com/YinhuaChen-cloud/fast

A LLVM PASS used to mutate on C/C++ code

I finish this LLVM PASS based on others' framework, this is a related video: https://www.youtube.com/watch?v=ar7cJl2aBuU&list=RDCMUCv2_41bSAa5Y_8BacJUZfjQ&start_radio=1

## How to compile

To compile LLVM PASS, you need llvm-project first

If you do not have llvm-project in your own local machine, do the following:
(NOTE: the following commands may fail several times because of lack of memory, just run them multiple times) (llvm-project may occupy 60G disk space after being compiled)

```
cd <where-you-want-to-compile-llvm>
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout llvmorg-9.0.0
git switch -c fast
cmake -S llvm -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_PROJECTS='clang'
ninja -C build check-llvm
echo "export LLVM_DIR=$(pwd)/build" >> ~/.bashrc  (if you use zsh, then switch to .zshrc)
source ~/.bashrc
```

If you do have llvm-project in your own machine, make sure it is compiled and there is build/bin/opt in llvm-project directory. And I use llvm-9.0.0 so you'd better use llvm-9.0.0 too.

set environment variable LLVM_DIR as follows:

```
echo "export LLVM_DIR=$(pwd)/build" >> ~/.bashrc  (if you use zsh, then switch to .zshrc)
source ~/.bashrc
```

After set environment variable LLVM_DIR, we can use the following commands to compile LLVM PASS

```
cd <where-you-want-to-compile-llvm>
clone this repo
cd this repo
mkdir -p build
cmake -GNinja -DLT_LLVM_INSTALL_DIR=$LLVM_DIR ..   
ninja
```

If you compile this project successfully, you will find the target dynamic lib: build/lib/libInjectFuncCall.so

This is the LLVM PASS used to do mutation on C/C++ code

Use the following command to use it:

```
$LLVM_DIR/bin/opt -load <path-to>/libInjectFuncCall.so -legacy-inject-func-call old.bc -o new.bc
```


