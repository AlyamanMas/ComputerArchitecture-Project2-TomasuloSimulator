#include <cctype>
#include <exception>
#include <string>
#include <vector>

#include "Instruction.hpp"

using namespace std;

std::vector<Token> tokenize(const std::string &line) {
  std::vector<Token> tokens;
  std::istringstream iss(line);
  std::string token;

  while (iss >> token) {
    if (token == "LOAD" || token == "STORE" || token == "BEQ" ||
        token == "CALL" || token == "RET" || token == "ADD" ||
        token == "ADDI" || token == "NAND" || token == "MUL") {
      tokens.push_back({TokenType::Instruction, token});
    } else if (token.at(0) == 'r' && std::isdigit(token.at(1))) {
      tokens.push_back({TokenType::Register, token});
    } else if (token.find('(') != std::string::npos) {
      // Split the offset from the register in the format offset(reg)
      size_t startParen = token.find('(');
      size_t endParen = token.find(')');
      if (startParen != std::string::npos && endParen != std::string::npos) {
        std::string offset = token.substr(0, startParen);
        std::string reg =
            token.substr(startParen + 1, endParen - startParen - 1);
        tokens.push_back({TokenType::Offset, offset});
        tokens.push_back({TokenType::RegisterInOffset, reg});
      }
    } else if (std::isdigit(token.at(0)) || token.at(0) == '+' ||
               token.at(0) == '-') {
      tokens.push_back({TokenType::Number, token});
    } else if (token.back() == ':') {
      // Remove the colon and classify as Label
      token.pop_back(); // Remove the last character, the colon
      tokens.push_back({TokenType::Label, token});
    } else {
      throw logic_error("Invalid token");
    }
  }

  return tokens;
}

void print_tokens(const std::vector<Token> &tokens) {
  for (const auto &token : tokens) {
    std::cout << "Type: ";
    switch (token.type) {
    case TokenType::Instruction:
      std::cout << "Instruction";
      break;
    case TokenType::Register:
      std::cout << "Register";
      break;
    case TokenType::Number:
      std::cout << "Number";
      break;
    case TokenType::Label:
      std::cout << "Label";
      break;
    case TokenType::Offset:
      std::cout << "Offset";
      break;
    case TokenType::RegisterInOffset:
      std::cout << "Register in Offset";
      break;
    default:
      std::cout << "Unknown";
      break;
    }
    std::cout << ", Value: " << token.value << std::endl;
  }
}
