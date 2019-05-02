#include <cctype>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

void error(char const* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/* 字句解析 */
enum class TokenType{
  PLUS,
  MINUS,
  ASTERISK,
  SLASH,
  NUMBER,
  BEGIN_PAREN,
  END_PAREN,
  GREATER_THAN_OR_EQUAL,
  LESS_THAN_OR_EQUAL,
  GREATER_THAN,
  LESS_THAN,
  NOT_EQUAL,
  EQUAL,
  EOS
};
struct Token{
  TokenType type;
  int value;
  char const* input;
  Token(TokenType type, char const* input): type(type), input(input){}
  Token(TokenType type, int value, char const* input): type(type), value(value), input(input){}
};
std::vector<Token> tokenize(char const* p){
  std::vector<Token> tokens;

  while(*p){
    if(std::isspace(*p)){
      p++;
      continue;
    }

    if(std::strncmp(p, ">=", 2) == 0){
      tokens.emplace_back(TokenType::GREATER_THAN_OR_EQUAL, p);
      p += 2;
      continue;
    }
    if(std::strncmp(p, "<=", 2) == 0){
      tokens.emplace_back(TokenType::LESS_THAN_OR_EQUAL, p);
      p += 2;
      continue;
    }
    if(std::strncmp(p, "!=", 2) == 0){
      tokens.emplace_back(TokenType::NOT_EQUAL, p);
      p += 2;
      continue;
    }
    if(std::strncmp(p, "==", 2) == 0){
      tokens.emplace_back(TokenType::EQUAL, p);
      p += 2;
      continue;
    }

    if(*p == '+'){
      tokens.emplace_back(TokenType::PLUS, p);
      p++;
      continue;
    }
    if(*p == '-'){
      tokens.emplace_back(TokenType::MINUS, p);
      p++;
      continue;
    }
    if(*p == '*'){
      tokens.emplace_back(TokenType::ASTERISK, p);
      p++;
      continue;
    }
    if(*p == '/'){
      tokens.emplace_back(TokenType::SLASH, p);
      p++;
      continue;
    }
    if(*p == '('){
      tokens.emplace_back(TokenType::BEGIN_PAREN, p);
      p++;
      continue;
    }
    if(*p == ')'){
      tokens.emplace_back(TokenType::END_PAREN, p);
      p++;
      continue;
    }
    if(*p == '<'){
      tokens.emplace_back(TokenType::LESS_THAN, p);
      p++;
      continue;
    }
    if(*p == '>'){
      tokens.emplace_back(TokenType::GREATER_THAN, p);
      p++;
      continue;
    }


    if(std::isdigit(*p)){;
      auto const p_tmp = p;
      tokens.emplace_back(TokenType::NUMBER, std::strtol(p, (char**)&p, 10), p_tmp);
      continue;
    }
    error("トークナイズできません: %s", p);
    exit(1);
  }

  tokens.emplace_back(TokenType::EOS, p);

  return tokens;
}

/* 構文解析 */
enum class ASTNodeType{
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
  BINARY_LESS_THAN,
  BINARY_LESS_THAN_OR_EQUAL,
  BINARY_EQUAL,
  BINARY_NOT_EQUAL,
  NUMBER
};
struct ASTNode{
  ASTNodeType type;
  ASTNode const* lhs;
  ASTNode const* rhs;
  int value;
};
ASTNode* newNodeBinary(
    ASTNodeType type,
    ASTNode const* lhs,
    ASTNode const* rhs){
  ASTNode* node = new ASTNode();
  node->type = type;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}
ASTNode* newNodeNumber(
    int value){
  ASTNode* node = new ASTNode();
  node->type = ASTNodeType::NUMBER;
  node->value = value;
  return node;
}
bool consume(TokenType type, std::vector<Token>::const_iterator& token_itr){
  if(token_itr -> type != type){
    return false;
  }
  ++token_itr;
  return true;
}
ASTNode* equality(std::vector<Token>::const_iterator& token_itr);
ASTNode* relational(std::vector<Token>::const_iterator& token_itr);
ASTNode* add(std::vector<Token>::const_iterator& token_itr);
ASTNode* mul(std::vector<Token>::const_iterator& token_itr);
ASTNode* term(std::vector<Token>::const_iterator& token_itr);
ASTNode* unary(std::vector<Token>::const_iterator& token_itr);
ASTNode* equality(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = relational(token_itr);
  for(;;){
    if(consume(TokenType::EQUAL, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_EQUAL, node, relational(token_itr));
    }else if(consume(TokenType::NOT_EQUAL, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_NOT_EQUAL, node, relational(token_itr));
    }else{
      return node;
    }
  }
}
ASTNode* relational(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = add(token_itr);
  for(;;){
    if(consume(TokenType::LESS_THAN_OR_EQUAL, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_LESS_THAN_OR_EQUAL, node, add(token_itr));
    }else if(consume(TokenType::GREATER_THAN_OR_EQUAL, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_LESS_THAN_OR_EQUAL, add(token_itr), node);  // swapped lhs, rhs
    }else if(consume(TokenType::LESS_THAN, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_LESS_THAN, node, add(token_itr));
    }else if(consume(TokenType::GREATER_THAN, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_LESS_THAN, add(token_itr), node);  // swapped lhs, rhs
    }else{
      return node;
    }
  }
}
ASTNode* add(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = mul(token_itr);
  for(;;){
    if(consume(TokenType::PLUS, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_ADD, node, mul(token_itr));
    }else if(consume(TokenType::MINUS, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_SUB, node, mul(token_itr));
    }else{
      return node;
    }
  }
}
ASTNode* mul(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = unary(token_itr);
  for(;;){
    if(consume(TokenType::ASTERISK, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_MUL, node, unary(token_itr));
    }else if(consume(TokenType::SLASH, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_DIV, node, unary(token_itr));
    }else{
      return node;
    }
  }
}
ASTNode* unary(std::vector<Token>::const_iterator& token_itr){
  if(consume(TokenType::PLUS, token_itr)){
    return term(token_itr);
  }else if(consume(TokenType::MINUS, token_itr)){
    auto const zeroNode = newNodeNumber(0);
    auto const subtractionNode = newNodeBinary(ASTNodeType::BINARY_SUB, zeroNode, term(token_itr));
    return subtractionNode;
  }else{
    return term(token_itr);
  }
}
ASTNode* term(std::vector<Token>::const_iterator& token_itr){
  // ( equality )
  if(consume(TokenType::BEGIN_PAREN, token_itr)){
    ASTNode* node = equality(token_itr);
    if(!consume(TokenType::END_PAREN, token_itr)){
      error("閉じカッコ ) がない: %s", token_itr -> input);
    }
    return node;
  }
  // number
  if(token_itr -> type == TokenType::NUMBER){
    ASTNode* node = newNodeNumber(token_itr->value);
    ++token_itr;
    return node;
  }

  error("数値か開きカッコ ( を期待したが、数値ではない: %s", token_itr -> input);
  return nullptr;
}


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

