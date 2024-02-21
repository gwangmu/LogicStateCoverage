MAKEFLAGS += --no-print-directory

ifdef LLVM_ROOT
	LLVM_BIN_DIR = ${LLVM_ROOT}/bin
	LLVM_LIB_DIR = ${LLVM_ROOT}/lib
	LLVM_INCLUDE_DIR = ${LLVM_ROOT}/include
else
	LLVM_BIN_DIR = $(shell llvm-config --bindir)
	LLVM_LIB_DIR = $(shell llvm-config --libdir)
	LLVM_INCLUDE_DIR = $(shell llvm-config --includedir)
endif

ifdef WRAP_CC
	WRAP_CC_PATH = ${WRAP_CC}
else
	WRAP_CC_PATH = ${LLVM_BIN_DIR}/clang
endif

ifdef WRAP_CXX
	WRAP_CXX_PATH = ${WRAP_CXX}
else
	WRAP_CXX_PATH = ${LLVM_BIN_DIR}/clang++
endif

CUR_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := ${CUR_DIR}/build
NPROC := 1 

all: ${BUILD_DIR}
ifndef LLVM_ROOT
ifeq (,$(shell which llvm-config))
	$(error No llvm-config found. Install llvm or set LLVM_ROOT.)
endif
endif
ifndef WRAP_CC
	$(warning WRAP_CC_PATH undefined. Taking default clang...)
endif
ifndef WRAP_CXX
	$(warning WRAP_CXX_PATH undefined. Taking default clang++...)
endif
	@echo "info: building.."; 
	@make -C${BUILD_DIR} -j${NPROC} 

${BUILD_DIR}:
	@echo "info: launching cmake.."
	@mkdir -p ${BUILD_DIR} 
	@PATH=${LLVM_BIN_DIR}:${PATH} \
		LLVM_ROOT_DIR=${LLVM_BIN_DIR} \
		LLVM_LIBRARY_DIRS=${LLVM_LIB_DIR} \
		LLVM_INCLUDE_DIRS=${LLVM_INCLUDE_DIR} \
		CC=clang CXX=clang++ \
		cmake -S. -B${BUILD_DIR} \
			-DWRAP_CC_PATH=${WRAP_CC_PATH} \
			-DWRAP_CXX_PATH=${WRAP_CXX_PATH} \
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_CXX_FLAGS_DEBUG="-fno-rtti -fpic" \
			-Wno-dev

clean:
	rm -rf ${BUILD_DIR}
