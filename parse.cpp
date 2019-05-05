#include "rvcc.hpp"

#include <algorithm>
#include <cctype>
#include <utility>
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

std::vector<std::pair<std::string, TokenType>> const static
  symbols_to_tokentype({
      {">=", TokenType::GREATER_THAN_OR_EQUAL},
      {"<=", TokenType::LESS_THAN_OR_EQUAL},
      {"!=", TokenType::NOT_EQUAL},
      {"==", TokenType::EQUAL},
      {"+", TokenType::PLUS},
      {"-", TokenType::MINUS},
      {"*", TokenType::ASTERISK},
      {"/", TokenType::SLASH},
      {"(", TokenType::BEGIN_PAREN},
      {")", TokenType::END_PAREN},
      {"{", TokenType::BEGIN_BRACE},
      {"}", TokenType::END_BRACE},
      {"<", TokenType::LESS_THAN},
      {">", TokenType::GREATER_THAN},
      {";", TokenType::SEMICOLON},
      {",", TokenType::COMMA},
      {"=", TokenType::ASSIGN}
    });
std::unordered_map<std::string, TokenType> const static
  keyword_to_tokentype({
    {"return", TokenType::RETURN},
    {"if",     TokenType::IF},
    {"for",    TokenType::FOR},
    {"while",  TokenType::WHILE}
    });

/* 字句解析 */
std::vector<Token> tokenize(char const* p){
  std::vector<Token> tokens;

  while(*p){
    if(std::isspace(*p)){
      p++;
      continue;
    }

    /* symbols tokens */
    if([&]()->bool{
      for(auto const& kvpair: symbols_to_tokentype){
        auto const& symbolstr = kvpair.first;
        if(std::strncmp(p, symbolstr.c_str(), symbolstr.size()) == 0){
          tokens.emplace_back(kvpair.second, p);
          p += symbolstr.size();
          return true;
        }
      }
      return false;
    }()){ continue; }

    /* keyword */
    if([&]()->bool{
      for(auto const& kvpair: keyword_to_tokentype){
        auto const& kw = kvpair.first;
        if(match_keyword(kw.c_str(), p)){
          tokens.emplace_back(kvpair.second, p);
          p += kw.size();
          return true;
        }
      }
      return false;
    }()){ continue; }


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
std::vector<ASTNode const*> block_items(std::vector<Token>::const_iterator& token_itr);
ASTNode* expression(std::vector<Token>::const_iterator& token_itr);
ASTNode* equality(std::vector<Token>::const_iterator& token_itr);
ASTNode* relational(std::vector<Token>::const_iterator& token_itr);
ASTNode* add(std::vector<Token>::const_iterator& token_itr);
ASTNode* mul(std::vector<Token>::const_iterator& token_itr);
ASTNode* term(std::vector<Token>::const_iterator& token_itr);
ASTNode* funccall(std::vector<Token>::const_iterator& token_itr);
std::vector<ASTNode const*> argument_list(std::vector<Token>::const_iterator& token_itr);
ASTNode* unary(std::vector<Token>::const_iterator& token_itr);
std::vector<ASTNode*> program(std::vector<Token>::const_iterator& token_itr){
  std::vector<ASTNode*> ret;
  while(token_itr->type != TokenType::EOS){
    ret.emplace_back(stmt(token_itr));
  }
  return ret;
}
void require_token(TokenType type, std::vector<Token>::const_iterator& token_itr){
  if(!consume(type, token_itr)){
    if(type == TokenType::IDENTIFIER){
      error("identifier が必要");
      exit(1);
    }
    for(auto const& kvpair: symbols_to_tokentype){
      if(kvpair.second == type){
        auto const required_token_string = kvpair.first;
        error("%s が必要", required_token_string.c_str());
        exit(1);
      }
    }
    error("TokenType::%d が必要", static_cast<int>(type));
    exit(1);
  }
}
ASTNode* stmt(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = new ASTNode();
  if(consume(TokenType::RETURN, token_itr)){
    // return expression;
    node->type = ASTNodeType::RETURN;
    node->lhs = expression(token_itr);
    require_token(TokenType::SEMICOLON, token_itr);
    return node;
  }else if(consume(TokenType::IF, token_itr)){
    // if ( expression ) stmt
    node->type = ASTNodeType::IF;
    require_token(TokenType::BEGIN_PAREN, token_itr);
    node->condition = expression(token_itr);
    require_token(TokenType::END_PAREN, token_itr);
    node->body = stmt(token_itr);
    return node;
  }else if(consume(TokenType::FOR, token_itr)){
    // for ( expr; expr; expr ) stmt
    node->type = ASTNodeType::FOR;
    require_token(TokenType::BEGIN_PAREN, token_itr);
    node->initialization = expression(token_itr);
    require_token(TokenType::SEMICOLON, token_itr);
    node->condition = expression(token_itr);
    require_token(TokenType::SEMICOLON, token_itr);
    node->afterthought = expression(token_itr);
    require_token(TokenType::END_PAREN, token_itr);
    node->body = stmt(token_itr);
    return node;
  }else if(consume(TokenType::WHILE, token_itr)){
    // while ( expression ) stmt
    node->type = ASTNodeType::WHILE;
    require_token(TokenType::BEGIN_PAREN, token_itr);
    node->condition = expression(token_itr);
    require_token(TokenType::END_PAREN, token_itr);
    node->body = stmt(token_itr);
    return node;
  }else if(consume(TokenType::BEGIN_BRACE, token_itr)){
    // { block_items }
    node->type = ASTNodeType::BLOCK;
    node->inner_nodes = block_items(token_itr);
    require_token(TokenType::END_BRACE, token_itr);
    return node;
  }else{
    node = expression(token_itr);
    require_token(TokenType::SEMICOLON, token_itr);
    return node;
  }
}
std::vector<ASTNode const*> block_items(std::vector<Token>::const_iterator& token_itr){
  std::vector<ASTNode const*> stmts;
  while(token_itr->type != TokenType::END_BRACE){
    stmts.push_back(stmt(token_itr));
  }
  return stmts;
}
ASTNode* expression(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = equality(token_itr);
  for(;;){
    if(consume(TokenType::ASSIGN, token_itr)){
      node = newNodeBinary(ASTNodeType::BINARY_ASSIGN, node, expression(token_itr));
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
    return funccall(token_itr);
  }else if(consume(TokenType::MINUS, token_itr)){
    auto const zeroNode = newNodeNumber(0);
    auto const subtractionNode = newNodeBinary(ASTNodeType::BINARY_SUB, zeroNode, funccall(token_itr));
    return subtractionNode;
  }else{
    return funccall(token_itr);
  }
}
ASTNode* funccall(std::vector<Token>::const_iterator& token_itr){
  ASTNode* node = term(token_itr);
  if(consume(TokenType::BEGIN_PAREN, token_itr)){
    // term ( argument_list|none )
    ASTNode* callnode = new ASTNode();
    callnode->type = ASTNodeType::FUNCTION_CALL;
    callnode->lhs = node;
    if(consume(TokenType::END_PAREN, token_itr)){
      // term ()
      callnode->inner_nodes = {};
    }else{
      // term ( argument_list )
      callnode->inner_nodes = argument_list(token_itr);
      require_token(TokenType::END_PAREN, token_itr);
    }
    return callnode;
  }
  return node;
}
std::vector<ASTNode const*> argument_list(std::vector<Token>::const_iterator& token_itr){
  std::vector<ASTNode const*> arguments;
  do{
    arguments.push_back(expression(token_itr));
  }while (consume(TokenType::COMMA, token_itr));
  return arguments;
}
ASTNode* term(std::vector<Token>::const_iterator& token_itr){
  // ( expression )
  if(consume(TokenType::BEGIN_PAREN, token_itr)){
    ASTNode* node = expression(token_itr);
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
