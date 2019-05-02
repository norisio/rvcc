#ifndef RVCC_HPP
#define RVCC_HPP

#include <vector>

void error(char const* fmt, ...);

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
  // IDENTIFIER,
  SEMICOLON,
  // ASSIGN,
  EOS
};
struct Token{
  TokenType type;
  int value;
  char const* input;
  Token(TokenType type, char const* input): type(type), input(input){}
  Token(TokenType type, int value, char const* input): type(type), value(value), input(input){}
};
std::vector<Token> tokenize(char const* p);


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
  // BINARY_ASSIGN,
  NUMBER
};
struct ASTNode{
  ASTNodeType type;
  ASTNode const* lhs;
  ASTNode const* rhs;
  int value;
};
std::vector<ASTNode*> program(std::vector<Token>::const_iterator& token_itr);

/* コード生成 */
void gen(ASTNode const* node);
#endif // RVCC_HPP
