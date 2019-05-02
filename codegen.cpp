#include "rvcc.hpp"

#include <iostream>


size_t static constexpr stack_size = 4;

void gen(ASTNode const* node){
  if(node->type == ASTNodeType::NUMBER){
    // push immediate
    std::cout <<
      "  addi sp, sp, -" << stack_size << "\n"
      "  li   a0, " << node->value << "\n"
      "  sw   a0, 0(sp)\n";
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  /* 二項演算子 */
  // pop 2 operands
  std::cout <<
    "  lw  a1, 0(sp)\n"
    "  lw  a0, " << stack_size << "(sp)\n"
    "  addi sp, sp, " <<  stack_size*2 << "\n";
  if(node->type == ASTNodeType::BINARY_ADD){
    std::cout << "  add a0, a0, a1\n";
  }else if(node->type == ASTNodeType::BINARY_SUB){
    std::cout << "  sub a0, a0, a1\n";
  }else if(node->type == ASTNodeType::BINARY_MUL){
    std::cout << "  mul a0, a0, a1\n";
  }else if(node->type == ASTNodeType::BINARY_DIV){
    std::cout << "  div a0, a0, a1\n";
  }else if(node->type == ASTNodeType::BINARY_LESS_THAN){
    std::cout << "  slt a0, a0, a1\n";
  }else if(node->type == ASTNodeType::BINARY_LESS_THAN_OR_EQUAL){
    std::cout <<
      "  sub a2, a0, a1\n"
      "  seqz a2, a2\n"
      "  slt a0, a0, a1\n"
      "  or  a0, a0, a2\n";
  }else if(node->type == ASTNodeType::BINARY_EQUAL){
    std::cout <<
      "  sub  a0, a0, a1\n"
      "  seqz a0, a0\n";
  }else if(node->type == ASTNodeType::BINARY_NOT_EQUAL){
    std::cout <<
      "  sub  a0, a0, a1\n"
      "  snez a0, a0\n";
  }else{
    error("不明なノード種別");
  }

  // push the result
  std::cout <<
    "  addi sp, sp, -" << stack_size << "\n"
    "  sw   a0, 0(sp)\n";

}
