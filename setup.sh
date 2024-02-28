#!/bin/bash

# Install LLVM 15.0.5

LLVM_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.5/clang+llvm-15.0.5-x86_64-linux-gnu-ubuntu-18.04.tar.xz
LLVM_TAR_NAME=${LLVM_URL##*/}

if [[ -d llvm ]]; then
  echo "error: directory 'llvm' already exists."
  exit
fi

wget $LLVM_URL && \
  mkdir llvm && \
  tar -xvf $LLVM_TAR_NAME -C llvm --strip-component 1 && \
  rm $LLVM_TAR_NAME
