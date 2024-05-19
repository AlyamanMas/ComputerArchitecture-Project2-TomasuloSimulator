#ifndef INCLUDE_SRC_INSTRUCTION_HPP_
#define INCLUDE_SRC_INSTRUCTION_HPP_

#include <cctype>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
using Address = uint16_t;
#define PC_width int(8);
enum class TokenType {
  Instruction,
  Register,
  Number,
  Label,
  Offset,
  RegisterInOffset,
  Identifier,
  issue,
  execute,
  write_result,
  reservation_station
};

struct Token {
  TokenType type;
  std::string value;
};

struct LoadInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg : 3;
  int offset : 5;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Load";
  std::optional<int> result;
  void execution() {
    // Example logic for execution, assuming some memory and registers are
    // accessible
    result = src_reg + offset; // Simplified example
  }
};

struct StoreInstruction {
  unsigned int src_reg : 3;
  unsigned int dest_reg : 3;
  int offset : 5;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Store";
  std::optional<int> result;
  void execution() {
    // Store logic here
  }
};

struct ConditionalBranchInstruction {
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
  // Since size of offset is not made very clear in the original document,
  // we use 5 bits in accordance with the previous type `LoadInstruction`
  int offset : 5;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "BEQ";
  std::optional<int> result;

  void execution() {
    // Branch logic here
  }
  unsigned int dest_reg : 3;
};

struct CallInstruction {
  // It does not seem very clear why we would use a 7-bit signed
  // offset over just using a string label. Since we are programming a
  // simulator, we can always map labels to PC values in a separate table and
  // use that to know where to branch.
  std::string label;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Call";
  std::optional<int> result;
  void execution() {
    // Call logic here
  }
  unsigned int dest_reg : 3;
};

struct RetInstruction {

  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Return";
  std::optional<int> result;
  void execution() {
    // Return logic here
  }
  unsigned int dest_reg : 3;
};

struct AddInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Add";
  std::optional<int> result;
  void execution() {
    // Example logic for addition
    result = 42; // Simplified example
  }
};

struct AddImmInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg : 3;
  int immediate : 5;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "AddI";
  std::optional<int> result;
  void execution() {
    // Immediate addition logic here
  }
};

struct NandInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "Nand";
  std::optional<int> result;
  void execution() {
    // NAND logic here
  }
};

struct MulInstruction {
  unsigned int dest_reg : 3;
  unsigned int src_reg1 : 3;
  unsigned int src_reg2 : 3;
  unsigned int issue : PC_width;
  unsigned int execute : PC_width;
  unsigned int write_result : PC_width;
  std::string reservation_station = "MUL";
  std::optional<int> result;
  void execution() {
    // Example logic for multiplication
    result = 42 * 42; // Simplified example
  }
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
