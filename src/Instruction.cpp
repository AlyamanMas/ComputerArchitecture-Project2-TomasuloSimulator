#include <cctype>
#include <cstdint>
#include <exception>
#include <format>
#include <stdexcept>
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

enum class ParserState {
  ExpectingInstructionOrLabel,
  ProcessingLoad,
  ProcessingStore,
  ProcessingConditionalBranch,
  ProcessingCall,
  ProcessingAdd,
  ProcessingAddImm,
  ProcessingNand,
  ProcessingMul,
};

namespace parser_internals {
void handle_instruction_type(ParserState &state, Address &pc,
                             const Token &token) {
  if (token.value == "LOAD") {
    state = ParserState::ProcessingLoad;
  } else if (token.value == "STORE") {
    state = ParserState::ProcessingStore;
  } else if (token.value == "BEQ") {
    state = ParserState::ProcessingConditionalBranch;
  } else if (token.value == "CALL") {
    state = ParserState::ProcessingCall;
  } else if (token.value == "RET") {
    pc += 2;
    state = ParserState::ExpectingInstructionOrLabel;
  } else if (token.value == "ADD") {
    state = ParserState::ProcessingAdd;
  } else if (token.value == "ADDI") {
    state = ParserState::ProcessingAddImm;
  } else if (token.value == "NAND") {
    state = ParserState::ProcessingNand;
  } else if (token.value == "MUL") {
    state = ParserState::ProcessingMul;
  } else {
    throw logic_error(
        format("Undefined behaviour: TokenType is Instruction but there is "
               "no code for handling instructions of type {}",
               token.value));
  }
}

void handle_load_instruction(vector<Instruction> &instructions,
                             Instruction &current_instruction,
                             ParserState &state, Address &pc,
                             uint8_t &argument_counter, int32_t &offset,
                             const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      throw logic_error(
          format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 3) {
      current_instruction = LoadInstruction{};
      get<LoadInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      throw logic_error(
          format("{} with size {} is not a valid register name at PC {}",
                 token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Offset) {
      throw logic_error(
          format("Expecting offset at {} but found {}", pc, token.value));
    }
    offset = stoi(token.value);
    if (offset > -17 && offset < 16) {
      get<LoadInstruction>(current_instruction).offset = offset;
      argument_counter += 1;
    } else {
      throw logic_error(
          format("{} is not a valid offset at PC {}", offset, pc));
    }
    break;
  case 2:
    if (token.type != TokenType::RegisterInOffset) {
      throw logic_error(
          format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<LoadInstruction>(current_instruction).src_reg =
          token.value.at(1) - '0';
      argument_counter = 0;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      throw logic_error(
          format("{} is not a valid register name at PC {}", token.value, pc));
    }
    break;
  }
}
} // namespace parser_internals

ParserResult parse(const std::vector<Token> &tokens) {
  vector<Instruction> instructions;
  Instruction current_instruction{};
  map<string, Address> labels;
  ParserState state = ParserState::ExpectingInstructionOrLabel;
  Address pc = 0;
  uint8_t argument_counter = 0;
  int32_t offset{};

  for (const auto &token : tokens) {
    switch (state) {
    case ParserState::ExpectingInstructionOrLabel:
      if (token.type == TokenType::Label) {
        labels.emplace(token.value, pc);
        break;
      }
      if (token.type != TokenType::Instruction) {
        throw logic_error(format("Error while parsing: expecting instruction "
                                 "but {} at PC {} is not a valid instruction.",
                                 token.value, pc));
      }
      parser_internals::handle_instruction_type(state, pc, token);
      break;
    case ParserState::ProcessingLoad:
      parser_internals::handle_load_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingStore:
      break;
    case ParserState::ProcessingConditionalBranch:
      break;
    case ParserState::ProcessingCall:
      break;
    case ParserState::ProcessingAdd:
      break;
    case ParserState::ProcessingAddImm:
      break;
    case ParserState::ProcessingNand:
      break;
    case ParserState::ProcessingMul:
      break;
    }
  }

  if (argument_counter != 0) {
    throw logic_error(
        format("The instruction at PC {} was not filled completely", pc));
  }

  return {instructions, labels};
};

void print_instruction(const Instruction &instr) {
  std::visit(InstructionPrinter{}, instr);
}
