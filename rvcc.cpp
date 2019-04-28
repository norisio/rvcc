#include <iostream>
#include <cstdlib>

int main(int argc, char** argv){
  if(argc != 2){
    std::cerr << "引数個数不正" << std::endl;
    return 1;
  }


  char* p = argv[1];

  std::cout <<
    ".global main\n"
    "\n"
    "main:\n"
    "  li   a0, " << std::strtol(p, &p, 10) << "\n";

  while(*p){
    if(*p == '+'){
      p++;
      std::cout << "  addi  a0, a0, " << std::strtol(p, &p, 10) << "\n";
      continue;
    }
    if(*p == '-'){
      p++;
      std::cout << "  addi  a0, a0, -" << std::strtol(p, &p, 10) << "\n";
      continue;
    }

    std::cerr << "予期しない文字です: " << (int)*p << std::endl;
    return 1;
  }


  std::cout <<
    "  ret\n" << std::endl;

  return 0;
}

