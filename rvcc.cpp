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

  std::vector<ASTNode*> code = program(token_itr);

  std::cout <<
    ".text\n"
    ".align 2\n"
    ".global main\n"
    "\n"
    "main:\n"
    << std::endl;

  size_t static constexpr stackframe_size = 100 * sizeof_variable;
  // prologue
  std::cout <<
    "  addi  sp, sp, -" << sizeof_variable << "\n"
    "  sd    s0, (sp)\n"
    "  mv    s0, sp\n"
    "  addi  sp, sp, -" << stackframe_size << "\n";

  for(ASTNode* const& stmtNode: code){
    gen(stmtNode);
    std::cout <<
      // pop the last result
      "  ld a0, 0(sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n";
  }


  // epilogue
  std::cout <<
    "  mv  sp, s0\n"
    "  ld  s0, (sp)\n"
    "  addi  sp, sp, " << sizeof_variable << "\n"
    "  ret\n" << std::endl;

  return 0;
}

