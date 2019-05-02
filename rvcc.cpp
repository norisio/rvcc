#include "rvcc.hpp"

#include <cstdarg>
#include <cstdio>
#include <iostream>

void error(char const* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(stderr, fmt, ap);
  std::fprintf(stderr, "\n");
  exit(1);
}






int main(int argc, char** argv){
  if(argc != 2){
    std::cerr << "引数個数不正" << std::endl;
    return 1;
  }


  auto const tokens = tokenize(argv[1]);

  auto token_itr = tokens.cbegin();
  ASTNode const* const astRoot = equality(token_itr);

  std::cout <<
    ".global main\n"
    "\n"
    "main:\n"
    << std::endl;

  gen(astRoot);


  std::cout <<
    "  lw a0, 0(sp)\n"
    "  ret\n" << std::endl;

  return 0;
}

