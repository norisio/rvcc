#include "rvcc.hpp"

#include <cctype>
#include <cstring>
#include <iostream>

bool is_alnum(char c){
  return false
    || ('a' <= c && c <= 'z')
    || ('A' <= c && c <= 'Z')
    || ('0' <= c && c <= '9')
    || (c == '_');
}

bool match_keyword(char const* kw, char const* p){
  auto const length = std::strlen(kw);
  return
    std::strncmp(p, kw, length) == 0
    && !is_alnum(*(p+length));
}

/* 字句解析 */
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
    if(*p == ';'){
      tokens.emplace_back(TokenType::SEMICOLON, p);
      p++;
      continue;
    }
    if(*p == '='){
      tokens.emplace_back(TokenType::ASSIGN, p);
      p++;
      continue;
    }

    /* keyword */
    if(match_keyword("return", p)){
      tokens.emplace_back(TokenType::RETURN, p);
      p += 6;
      continue;
    }

    /* identifier */
    if(std::isalpha(*p) || *p == '_'){
      auto p_lookahead = p+1;
      while(is_alnum(*p_lookahead)){
        ++p_lookahead;
      }
      Token id_token;
      id_token.type = TokenType::IDENTIFIER;
      id_token.input = p;
      ptrdiff_t const name_length = p_lookahead-p;
      id_token.id_name = std::string(p, name_length);
      tokens.push_back(id_token);
      p = p_lookahead;
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
void VariablesInfo::put(std::string const& id_name){
  if(this->offsets.find(id_name) == this->offsets.end()){
    this->num_of_found_variables += 1;
    this->offsets[id_name] = num_of_found_variables * sizeof_variable;
  }
}
VariablesInfo variables_info;
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
ASTNode* newNodeIdentifier(
    std::string const& name){
  ASTNode* node = new ASTNode();
  node->type = ASTNodeType::IDENTIFIER;
  node->id_name = name;
  return node;
}
bool consume(TokenType type, std::vector<Token>::const_iterator& token_itr){
  if(token_itr -> type != type){
    return false;
  }
  ++token_itr;
  return true;
}
ASTNode* stmt(std::vector<Token>::const_iterator& token_itr);
ASTNode* assignment(std::vector<Token>::const_iterator& token_itr);
ASTNode* equality(std::vector<Token>::const_iterator& token_itr);
ASTNode* relational(std::vector<Token>::const_iterator& token_itr);
ASTNode* add(std::vector<Token>::const_iterator& token_itr);
ASTNode* mul(std::vector<Token>::const_iterator& token_itr);
ASTNode* term(std::vector<Token>::const_iterator& token_itr);
ASTNode* unary(std::vector<Token>::const_iterator& token_itr);
std::vector<ASTNode*> program(std::vector<Token>::const_iterator& token_itr){
  std::vector<ASTNode*> ret;
  while(token_itr->type != TokenType::EOS){
    ret.emplace_back(stmt(token_itr));
  }
  return ret;
}
ASTNode* stmt(std::vector<Token>::const_iterator& token_itr){
  ASTNode* const node = [&](){
    if(consume(TokenType::RETURN, token_itr)){
      ASTNode* return_node = new ASTNode();
      return_node->type = ASTNodeType::RETURN;
      return_node->lhs = assignment(token_itr);
      return return_node;
    }else{
      return assignment(token_itr);
    }
  }();

  if(!consume(TokenType::SEMICOLON, token_itr)){
    error("セミコロン ; が無い");
    exit(1);
  }
  return node;
}
ASTNode* assignment(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = equality(token_itr);
  for(;;){
    if(consume(TokenType::ASSIGN, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_ASSIGN, node, assignment(token_itr));
    }else{
      return node;
    }
  }
}
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
  // ( assignment )
  if(consume(TokenType::BEGIN_PAREN, token_itr)){
    ASTNode* node = assignment(token_itr);
    if(!consume(TokenType::END_PAREN, token_itr)){
      error("閉じカッコ ) がない: %s", token_itr -> input);
      exit(1);
    }
    return node;
  }
  // number
  if(token_itr -> type == TokenType::NUMBER){
    ASTNode* node = newNodeNumber(token_itr->value);
    ++token_itr;
    return node;
  }
  // identifier
  if(token_itr -> type == TokenType::IDENTIFIER){
    ASTNode* node = newNodeIdentifier(token_itr->id_name);
    variables_info.put(node->id_name);
    ++token_itr;
    return node;
  }


  error("数値か開きカッコ ( を期待したが、数値ではない: %s", token_itr -> input);
  return nullptr;
}
