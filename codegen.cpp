#include "rvcc.hpp"

#include <iostream>
#include <cmath>
#include <array>

std::array<std::string, 8> const static
  argument_registers{ "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7" };

size_t static generating_funcdef_function_id = 0;

void gen_lvalue(ASTNode const* node){
  if(node->type != ASTNodeType::IDENTIFIER){
    error("代入式左辺が左辺値でない");
    exit(1);
  }

  std::string const id_name = node->id_name;

  ptrdiff_t const offset = variables_info.offset_of(generating_funcdef_function_id, id_name);
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
  if(node->type == ASTNodeType::FUNCTION_DEFINITION){
    // function label
    auto const name = node->id_name;
    std::cout <<
      "# BEGIN of " << name << "\n"
      ".text\n"
      ".align 2\n"
      ".global " << name << "\n"
      "\n"
      << name << ":\n" << std::endl;

    for(auto&& e: variables_info.offsets.at(generating_funcdef_function_id)){
      std::cout << "  # var " << e.first << "\t -> offset " << e.second << std::endl;
    }

    size_t const stackframe_size =
      variables_info.num_of_variables(generating_funcdef_function_id)
      * sizeof_variable;
    // prologue
    std::cout <<
      "# prologue\n"
      // preserve base pointer in stack
      "  addi  sp, sp, -" << sizeof_variable << "\n"
      "  sd    s0, (sp)\n"
      // new base pointer
      "  mv    s0, sp\n"
      // get stack frame
      "  addi  sp, sp, -" << stackframe_size << "\n";

    // copy parameters from arguments(in registers)
    size_t const num_of_parameters = node->call_list.size();
    for(size_t i=0; i<num_of_parameters; ++i){
      ASTNode const* const param = node->call_list.at(i);
      ptrdiff_t const offset = variables_info.offset_of(generating_funcdef_function_id, param->id_name);
      std::cout <<
        "  # copy " << param->id_name << "\n"
        "  sd   " << argument_registers.at(i) << ", -" << offset << "(s0)\n";
    }

    std::cout <<
      "\n# body\n";
    for(ASTNode const* const stmt: node->inner_nodes){
      gen(stmt);
      /*
      std::cout <<
        // pop the last result
        "  ld a0, 0(sp)\n"
        "  addi  sp, sp, " << sizeof_variable << "\n";
        */
    }


    // epilogue
    std::cout <<
      "# epilogue\n"
      "  mv  sp, s0\n"
      "  ld  s0, (sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n"
      "  ret\n"
      "# END of " << name << "\n\n" << std::endl;

    generating_funcdef_function_id++;
    return;
  }

  if(node->type == ASTNodeType::NUMBER){
    // push immediate
    std::cout <<
      "  li   a0, " << node->value << "\n"
      "  addi sp, sp, -" << sizeof_variable << "\n"
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

  if(node->type == ASTNodeType::IF){
    size_t static new_if_stmt_sequence = 0;
    auto const if_stmt_sequence = new_if_stmt_sequence;
    new_if_stmt_sequence++;
    std::string const label = ".IF_STMT_" + std::to_string(if_stmt_sequence) + "_ELSE";

    gen(node->condition);
    std::cout <<
      // pop the evaluated condition
      "  ld  a0, (sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n"
      "  beqz  a0, " << label << "\n";
    gen(node->body);
    // write label
    std::cout << label << ":\n";
    return;
  }

  if(node->type == ASTNodeType::FOR){
    size_t static new_for_stmt_sequence = 0;
    auto const for_stmt_sequence = new_for_stmt_sequence;
    new_for_stmt_sequence++;
    std::string const loop_label = ".FOR_STMT_" + std::to_string(for_stmt_sequence) + "_LOOP";
    std::string const break_label = ".FOR_STMT_" + std::to_string(for_stmt_sequence) + "_BREAK";

    gen(node->initialization);
    std::cout << loop_label << ":\n";
    gen(node->condition);
    std::cout <<
      // pop the evaluated condition
      "  ld  a0, (sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n"
      "  beqz  a0, " << break_label << "\n";
    gen(node->body);
    gen(node->afterthought);
    std::cout <<
      "  j   " << loop_label << "\n"
      << break_label << ":\n";

    return;
  }

  if(node->type == ASTNodeType::WHILE){
    size_t static new_while_stmt_sequence = 0;
    auto const while_stmt_sequence = new_while_stmt_sequence;
    new_while_stmt_sequence++;
    std::string const loop_label = ".WHILE_STMT_" + std::to_string(while_stmt_sequence) + "_LOOP";
    std::string const break_label = ".WHILE_STMT_" + std::to_string(while_stmt_sequence) + "_BREAK";

    std::cout << loop_label << ":\n";
    gen(node->condition);
    std::cout <<
      // pop the evaluated condition
      "  ld  a0, (sp)\n"
      "  addi  sp, sp, " << sizeof_variable << "\n"
      "  beqz  a0, " << break_label << "\n";
    gen(node->body);
    std::cout <<
      "  j   " << loop_label << "\n"
      << break_label << ":\n";

    return;
  }

  if(node->type == ASTNodeType::BLOCK){
    for(ASTNode const* const stmt_node: node->inner_nodes){
      gen(stmt_node);
      if(true
          && stmt_node->type != ASTNodeType::BLOCK
          && stmt_node->type != ASTNodeType::IF
          && stmt_node->type != ASTNodeType::FOR
          && stmt_node->type != ASTNodeType::WHILE
          ){
        std::cout <<
          "  addi  sp, sp, " << sizeof_variable << " # discard the stmt's result \n";
      }
    }
    return;
  }

  if(node->type == ASTNodeType::FUNCTION_CALL){
    ASTNode const* const callee = node -> lhs;
    if(callee->type == ASTNodeType::IDENTIFIER){
      size_t const num_of_args = std::min(argument_registers.size(), node->call_list.size());
      for(size_t i=0; i<num_of_args; ++i){
        // evaluate arguments
        gen(node->call_list.at(i));
      }
      for(size_t i=0; i<num_of_args; ++i){
        // load arguments
        ptrdiff_t const offset = sizeof_variable * (num_of_args-i-1);
        std::cout <<
          "  ld   " << argument_registers.at(i) << ", " << offset << "(sp)\n";
      }
      std::cout <<
        // pop values for arguments from stack
        "  addi  sp, sp, " <<  sizeof_variable * num_of_args << "\n"
        // save return address
        "  addi  sp, sp, -" << sizeof_variable << "\n"
        "  sd    ra, (sp)\n"
        "  call  ra, " << callee->id_name << "\n"
        // restore ra
        "  ld    ra, (sp)\n"
        // save returned a0 at the top of the stack
        "  sd    a0, (sp)\n";
      return;
    }else{
      error("呼び出せないtermに()がついている");
      exit(1);
    }
  }


  /* 二項演算子 */
  gen(node->lhs);
  gen(node->rhs);

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
  }else if(node->type == ASTNodeType::BINARY_MOD){
    std::cout << "  rem a0, a0, a1\n";
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
