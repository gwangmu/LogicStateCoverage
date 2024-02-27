### Requirement

 - LLVM 15.0.5+
 - CMake 3.16.3+ 

### How to Use

 1. Make (binaries in `build`).
 2. Compile the fuzzing target with `lscov-clang`.
 3. Start off `lscov-daemon` in Terminal 1.
 4. Start fuzzing in Terminal 2.
 5. Upon the end of fuzzing, turn off `lscov-daemon`.
 6. Check logic state coverage in `lscov.out`.
