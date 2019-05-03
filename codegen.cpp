#include "rvcc.hpp"

#include <iostream>


void gen_lvalue(ASTNode const* node){
  if(node->type != ASTNodeType::IDENTIFIER){
    error("代入式左辺が左辺値でない");
    exit(1);
  }

  char const id_name = node->id_name;

  size_t const offset = ('z' - id_name + 1) * sizeof_variable;
  std::cout <<
    // calculate local variable's address
    "  mv   a0, s0\n"
    "  addi a0, a0, -" << offset << "\n"
    // push it
    "  addi sp, sp, -" << sizeof_variable << "\n"
    "  sd   a0, (sp)\n";
  return;
}

void gen(ASTNode const* node){
  if(node->type == ASTNodeType::NUMBER){
    // push immediate
    std::cout <<
      "  addi sp, sp, -" << sizeof_variable << "\n"
      "  li   a0, " << node->value << "\n"
      "  sd   a0, 0(sp)\n";
    return;
  }

  if(node->type == ASTNodeType::IDENTIFIER){
    gen_lvalue(node);
    std::cout <<
      // pop 1 operand
      "  ld  a0, (sp)\n"
      // dereference
      "  ld  a0, (a0)\n"
      // overwrite M[sp]
      "  sd  a0, (sp)\n";
    return;
  }

  if(node->type == ASTNodeType::BINARY_ASSIGN){
    gen_lvalue(node->lhs);
    gen(node->rhs);
    std::cout <<
      // pop 2 operands (a0: an address to be assigned, a1: a value to assign)
      "  ld  a1, (sp)\n"
      "  ld  a0, " << sizeof_variable << "(sp)\n"
      "  addi sp, sp, " <<  sizeof_variable*2 << "\n"
      // assign a1 to M[a0]
      "  sd  a1, (a0)\n"
      // push the result to stack
      "  addi sp, sp, -" << sizeof_variable << "\n"
      "  sd  a1, (sp)\n";
    return;
  }

  if(node->type == ASTNodeType::RETURN){
    gen(node->lhs);
    std::cout <<
      "  mv  sp, s0\n"
      "  ld  s0, (sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n"
      "  ret\n" << std::endl;
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  /* 二項演算子 */
  // pop 2 operands
  std::cout <<
    "  ld  a1, 0(sp)\n"
    "  ld  a0, " << sizeof_variable << "(sp)\n"
    "  addi sp, sp, " <<  sizeof_variable*2 << "\n";
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
    "  addi sp, sp, -" << sizeof_variable << "\n"
    "  sd   a0, 0(sp)\n";

}
