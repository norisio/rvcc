#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>

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
  EOS
};
struct Token{
  TokenType type;
  int value;
  char const* input;
};
std::vector<Token> tokenize(char const* p){
  std::vector<Token> tokens;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+'){
      Token token;
      token.type = TokenType::PLUS;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == '-'){
      Token token;
      token.type = TokenType::MINUS;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == '*'){
      Token token;
      token.type = TokenType::ASTERISK;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == '/'){
      Token token;
      token.type = TokenType::SLASH;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == '('){
      Token token;
      token.type = TokenType::BEGIN_PAREN;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == ')'){
      Token token;
      token.type = TokenType::END_PAREN;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }

    if(isdigit(*p)){;
      Token token;
      token.type = TokenType::NUMBER;
      token.input = p;
      token.value = std::strtol(p, (char**)&p, 10);
      tokens.push_back(token);
      continue;
    }
    error("トークナイズできません: %s", p);
    exit(1);
  }

  Token token;
  token.type = TokenType::EOS;
  token.input = p;
  tokens.push_back(token);

  return tokens;
}

/* 構文解析 */
enum class ASTNodeType{
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
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
ASTNode* mul(std::vector<Token>::const_iterator& token_itr);
ASTNode* term(std::vector<Token>::const_iterator& token_itr);
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
  ASTNode* node = term(token_itr);
  for(;;){
    if(consume(TokenType::ASTERISK, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_MUL, node, term(token_itr));
    }else if(consume(TokenType::SLASH, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_DIV, node, term(token_itr));
    }else{
      return node;
    }
  }
}
ASTNode* term(std::vector<Token>::const_iterator& token_itr){
  // ( add )
  if(consume(TokenType::BEGIN_PAREN, token_itr)){
    ASTNode* node = add(token_itr);
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
  ASTNode const* const astRoot = add(token_itr);

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

