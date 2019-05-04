#ifndef RVCC_HPP
#define RVCC_HPP

#include <vector>
#include <string>
#include <unordered_map>

size_t static constexpr sizeof_variable = 8;

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
  IDENTIFIER,
  SEMICOLON,
  ASSIGN,
  RETURN,
  IF,
  EOS
};
struct Token{
  TokenType type;
  int value;           // type==NUMBER
  std::string id_name; // type==IDENTIFIER
  char const* input;
  Token() = default;
  Token(Token const& rhs) = default;
  Token& operator=(Token const& rhs) = default;
  Token(TokenType type, char const* input): type(type), input(input){}
  Token(TokenType type, int value, char const* input): type(type), value(value), input(input){}
  ~Token(){}
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
  BINARY_ASSIGN,
  NUMBER,
  IDENTIFIER,
  IF,
  RETURN
};
struct ASTNode{
  ASTNodeType type;
  ASTNode const* lhs;
  ASTNode const* rhs;
  ASTNode const* condition;
  ASTNode const* body;
  int value;    // type==NUMBER
  std::string id_name; // type==IDENTIFIER
};
std::vector<ASTNode*> program(std::vector<Token>::const_iterator& token_itr);

class VariablesInfo{
  size_t num_of_found_variables;
  std::unordered_map<std::string, ptrdiff_t> offsets;
public:
  VariablesInfo():num_of_found_variables(0ul){}
  void put(std::string const& id_name);
  ptrdiff_t offset_of(std::string const& id_name){return offsets[id_name];}
};
extern VariablesInfo variables_info;


/* コード生成 */
void gen(ASTNode const* node);
#endif // RVCC_HPP
