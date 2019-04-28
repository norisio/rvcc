#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdarg>

enum class TokenType{
  OPERATOR_PLUS,
  OPERATOR_MINUS,
  NUMBER,
  EOS
};
struct Token{
  TokenType type;
  int value;
  char const* input;
};

void error(char const* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

std::vector<Token> tokenize(char const* p){
  std::vector<Token> tokens;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+'){
      Token token;
      token.type = TokenType::OPERATOR_PLUS;
      token.input = p;
      tokens.push_back(token);
      p++;
      continue;
    }
    if(*p == '-'){
      Token token;
      token.type = TokenType::OPERATOR_MINUS;
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

int main(int argc, char** argv){
  if(argc != 2){
    std::cerr << "引数個数不正" << std::endl;
    return 1;
  }


  auto const tokens = tokenize(argv[1]);

  std::cout <<
    ".global main\n"
    "\n"
    "main:\n"
    << std::endl;

  if(tokens.at(0).type != TokenType::NUMBER){
    error("最初の項がNUMBERでない");
  }
  std::cout << "  li  a0, " << tokens.at(0).value << "\n";

  for(size_t i=1; i<tokens.size() && tokens.at(i).type != TokenType::EOS; ){
    if(tokens.at(i).type == TokenType::OPERATOR_PLUS){
      i++;
      if(tokens.at(i).type != TokenType::NUMBER){
        error("予期しないトークン: %s", tokens.at(i).input);
      }
      std::cout << "  addi a0, a0, " << tokens.at(i).value << "\n";
      i++;
      continue;
    }

    if(tokens.at(i).type == TokenType::OPERATOR_MINUS){
      i++;
      if(tokens.at(i).type != TokenType::NUMBER){
        error("予期しないトークン: %s", tokens.at(i).input);
      }
      std::cout << "  addi a0, a0, -" << tokens.at(i).value << "\n";
      i++;
      continue;
    }

    error("予期しないトークン: %s", tokens.at(i).input);
  }


  std::cout <<
    "  ret\n" << std::endl;

  return 0;
}

