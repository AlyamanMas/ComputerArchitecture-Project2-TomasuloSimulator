#include "Instruction.hpp"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

int main() {
  string prog;
  stringstream prog_stream;
  prog_stream << cin.rdbuf();
  prog = prog_stream.str();
  vector<Token> tokens = tokenize(prog);
  print_tokens(tokens);
  auto [instructions, labels, pc] = parse(tokens);
  for (auto i : instructions) {
    print_instruction(i);
  }
  cout << "PC is: " << pc << endl;

  return 0;
}
