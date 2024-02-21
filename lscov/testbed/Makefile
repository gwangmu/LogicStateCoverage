ifdef LLVM_ROOT
	LLVM_BIN_DIR = ${LLVM_ROOT}/bin
	LLVM_LIB_DIR = ${LLVM_ROOT}/lib
	LLVM_INCLUDE_DIR = ${LLVM_ROOT}/include
else
ifeq (,$(shell llvm-config --version))
$(error "No llvm-config found. Install llvm or set LLVM_ROOT.")
endif
	LLVM_BIN_DIR = $(shell llvm-config --bindir)
	LLVM_LIB_DIR = $(shell llvm-config --libdir)
	LLVM_INCLUDE_DIR = $(shell llvm-config --includedir)
endif
	
CUR_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR := ${CUR_DIR}/build
NPROC := 4

all: ${BUILD_DIR}
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
			-DCMAKE_BUILD_TYPE=Debug \
			-DCMAKE_CXX_FLAGS_DEBUG="-fno-rtti -fpic"

clean:
	rm -rf ${BUILD_DIR}
