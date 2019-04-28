#include <iostream>

int main(int argc, char** argv){
  if(argc != 2){
    std::cerr << "引数個数不正" << std::endl;
    return 1;
  }

  std::cout <<
    ".global main\n"
    "\n"
    "main:\n"
    "  li   a0, " << argv[1] << "\n"
    "  ret\n" << std::endl;

  return 0;
}

