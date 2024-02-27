#!/bin/bash

# Install LLVM 15.0.5

LLVM_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.5/clang+llvm-15.0.5-x86_64-linux-gnu-ubuntu-18.04.tar.xz
LLVM_TAR_NAME=${LLVM_URL##*/}

echo $LLVM_TAR_NAME
exit

wget $LLVM_URL
tar -xvf $LLVM_TAR_NAME llvm
rm $LLVM_TAR_NAME
