/*
 * lscov - clang wrapper
 * ---------------------
 * 
 * Mostly based on AFL. (https://github.com/google/AFL)
 * See "llvm_mode/afl-clang-fast.c" for the original implementation.
 */

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char** argv) {
  char** cc_params = (char**)malloc((argc + 128) * sizeof(char*));
  int cc_par_cnt = 1;

  char _exepath[512];
  int _len = readlink("/proc/self/exe", _exepath, sizeof(_exepath)-1);
  if (_len < 0) {
    std::cerr << "fatal: fail to resolve executable path.\n";
    abort();
  }

  std::string exepath = std::string(_exepath);

  int _name_pos = exepath.rfind('/');
  std::string exename = exepath.substr(_name_pos + 1); 
  std::string basepath = exepath.substr(0, _name_pos);

  std::string wrap_exepath;
  if (exename.length() >= 2 && exename.substr(exename.length() - 2) == "++")
    wrap_exepath = basepath + "/wrap-clang++";
  else
    wrap_exepath = basepath + "/wrap-clang";

  std::string real_exepath;

  char _buf[1024] = {0};
  _len = readlink(wrap_exepath.c_str(), _buf, sizeof(_buf)-1);
  if (_len >= 0) 
    real_exepath = std::string(_buf);
  else {
    std::cerr << "fatal: invalid wrapped CC/CXX path.\n";
    abort();
  }
  
  cc_params[0] = (char*)real_exepath.c_str();

  std::string clang_chk = "which " + real_exepath + " >/dev/null 2>&1";
  if (system(clang_chk.c_str())) {
    std::cerr << "fatal: fail to find '" << real_exepath << "'.";
    abort();
  }

  std::string _libpath = std::string(basepath + "/libLSCovPass.so");
  std::string pass_plugin = "-fpass-plugin=" + _libpath;
  std::string rt_obj = std::string(basepath + "/libLSCovRT.a");
  //std::string rt_obj = std::string(basepath + "/CMakeFiles/LSCovRT.dir/lscov-llvm-rt.a.c.o");

  while (--argc) {
    char* cur = *(++argv);
    cc_params[cc_par_cnt++] = cur;
  }

  cc_params[cc_par_cnt++] = (char*)"-Xclang";
  cc_params[cc_par_cnt++] = (char*)pass_plugin.c_str();
  cc_params[cc_par_cnt++] = (char*)rt_obj.c_str();
  cc_params[cc_par_cnt++] = (char*)"-lpthread";
  cc_params[cc_par_cnt] = NULL;

  execvp(cc_params[0], (char**)cc_params);

  return 0;
}
