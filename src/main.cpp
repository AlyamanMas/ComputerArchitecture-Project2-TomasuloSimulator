#include "Instruction.hpp"
#include <iostream>
#include <vector>

int main() {
  std::string line;
  while (getline(std::cin, line)) {
    if (line.empty())
      continue;
    std::vector<Token> tokens = tokenize(line);
    print_tokens(tokens);
  }
  return 0;
}
