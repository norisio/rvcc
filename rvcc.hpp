#ifndef RVCC_HPP
#define RVCC_HPP

#include <vector>
#include <string>
#include <unordered_map>

size_t static constexpr sizeof_variable = 16;

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
  BEGIN_BRACE,
  END_BRACE,
  GREATER_THAN_OR_EQUAL,
  LESS_THAN_OR_EQUAL,
  GREATER_THAN,
  LESS_THAN,
  NOT_EQUAL,
  EQUAL,
  IDENTIFIER,
  SEMICOLON,
  COMMA,
  ASSIGN,
  RETURN,
  IF,
  FOR,
  WHILE,
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
  FOR,
  WHILE,
  BLOCK,
  FUNCTION_CALL,
  FUNCTION_DEFINITION,
  RETURN
};
struct ASTNode{
  ASTNodeType type;
  ASTNode const* lhs;
  ASTNode const* rhs;
  ASTNode const* initialization;
  ASTNode const* condition;
  ASTNode const* afterthought;
  ASTNode const* body;
  std::vector<ASTNode const*> call_list;
  std::vector<ASTNode const*> inner_nodes;
  int value;    // type==NUMBER
  std::string id_name; // type==IDENTIFIER
};
std::vector<ASTNode*> program(std::vector<Token>::const_iterator& token_itr);

class VariablesInfo{
public:
  std::vector<std::unordered_map<std::string, ptrdiff_t>> offsets;
  VariablesInfo(){}
  void put(size_t function_id, std::string const& id_name);
  ptrdiff_t offset_of(size_t function_id, std::string const& id_name){return offsets.at(function_id).at(id_name);}
  void function_exists(size_t function_id);
  size_t num_of_variables(size_t function_id){return offsets.at(function_id).size();}
};
extern VariablesInfo variables_info;


/* コード生成 */
void gen(ASTNode const* node);
#endif // RVCC_HPP
