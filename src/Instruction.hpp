#ifndef INCLUDE_SRC_INSTRUCTION_HPP_
#define INCLUDE_SRC_INSTRUCTION_HPP_

#include <cctype>
#include <cstdint>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

using Address = uint16_t;

enum class TokenType {
  Instruction,
  Register,
  Number,
  Label,
  Offset,
  RegisterInOffset,
  Identifier
};

struct Token {
  TokenType type;
  std::string value;
};

struct LoadInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg : 3;
  int offset : 5;
};

struct StoreInstruction {
  unsigned int src_reg : 3;
  unsigned int dest_reg : 3;
  int offset : 5;
};

struct ConditionalBranchInstruction {
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
  // Since size of offset is not made very clear in the original document,
  // we use 5 bits in accordance with the previous type `LoadInstruction`
  int offset : 5;
};

struct CallInstruction {
  // It does not seem very clear why we would use a 7-bit signed
  // offset over just using a string label. Since we are programming a
  // simulator, we can always map labels to PC values in a separate table and
  // use that to know where to branch.
  std::string label;
};

struct RetInstruction {};

struct AddInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
};

struct AddImmInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg : 3;
  int immediate : 5;
};

struct NandInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
};

struct MulInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
};

using Instruction =
    std::variant<LoadInstruction, StoreInstruction,
                 ConditionalBranchInstruction, CallInstruction, RetInstruction,
                 AddInstruction, AddImmInstruction, NandInstruction,
                 MulInstruction>;

/// @throws std::logic_error if it encounters undefined text
std::vector<Token> tokenize(const std::string &line);

void print_tokens(const std::vector<Token> &tokens);

struct ParserResult {
  std::vector<Instruction> instructions;
  std::map<std::string, Address> labels;
  Address pc;
};

ParserResult parse(const std::vector<Token> &tokens);

// Function to print any instruction
void print_instruction(const Instruction &instr);
#endif // INCLUDE_SRC_INSTRUCTION_HPP_
