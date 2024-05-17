#include <cctype>
#include <cstdint>
#include <exception>
// #include <format>
#include <stdexcept>
#include <string>
#include <vector>

#include "Instruction.hpp"

using namespace std;

// Make a new delimiter table for our tokenizer stream so
// we can treat comma as a delimiter
struct asm_whitespace : ctype<char> {
  static const mask *make_table() {
    // make a copy of the "C" locale table
    static vector<mask> v(classic_table(), classic_table() + table_size);
    v[','] |= space; // comma will be classified as whitespace
    // v[' '] &= ~space; // space will not be classified as whitespace
    return &v[0];
  }

  asm_whitespace(size_t refs = 0) : ctype(make_table(), false, refs) {}
};

vector<Token> tokenize(const string &line) {
  vector<Token> tokens;
  istringstream iss(line);
  iss.imbue(locale(iss.getloc(), new asm_whitespace));
  string token;

  while (iss >> token) {
    if (token == "LOAD" || token == "STORE" || token == "BEQ" ||
        token == "CALL" || token == "RET" || token == "ADD" ||
        token == "ADDI" || token == "NAND" || token == "MUL") {
      tokens.push_back({TokenType::Instruction, token});
    } else if (token.at(0) == 'r' && isdigit(token.at(1))) {
      tokens.push_back({TokenType::Register, token});
    } else if (token.find('(') != string::npos) {
      // Split the offset from the register in the format offset(reg)
      size_t startParen = token.find('(');
      size_t endParen = token.find(')');
      if (startParen != string::npos && endParen != string::npos) {
        string offset = token.substr(0, startParen);
        string reg = token.substr(startParen + 1, endParen - startParen - 1);
        tokens.push_back({TokenType::Offset, offset});
        tokens.push_back({TokenType::RegisterInOffset, reg});
      }
    } else if (isdigit(token.at(0)) || token.at(0) == '+' ||
               token.at(0) == '-') {
      tokens.push_back({TokenType::Number, token});
    } else if (token.back() == ':') {
      // Remove the colon and classify as Label
      token.pop_back(); // Remove the last character, the colon
      tokens.push_back({TokenType::Label, token});
    } else if (!isdigit(token.front())) {
      tokens.push_back({TokenType::Identifier, token});
    } else {
      throw logic_error("Invalid token");
    }
  }

  return tokens;
}

void print_tokens(const vector<Token> &tokens) {
  for (const auto &token : tokens) {
    switch (token.type) {
    case TokenType::Instruction:
      cout << "Instruction";
      break;
    case TokenType::Register:
      cout << "Register";
      break;
    case TokenType::Number:
      cout << "Number";
      break;
    case TokenType::Label:
      cout << "Label";
      break;
    case TokenType::Offset:
      cout << "Offset";
      break;
    case TokenType::RegisterInOffset:
      cout << "Register in Offset";
      break;
    case TokenType::Identifier:
      cout << "Identifier";
      break;
    }
    cout << " { " << token.value << " }" << endl;
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
                             const Token &token, map<string, Address> &labels,
                             vector<Instruction> &instructions) {
  if (token.type == TokenType::Label) {
    labels.emplace(token.value, pc);
    return;
  }
  if (token.type != TokenType::Instruction) {
    // throw logic_error(format("Error while parsing: expecting instruction "
                            //  "but {} at PC {} is not a valid instruction.",
                            //  token.value, pc));
  }
  if (token.value == "LOAD") {
    state = ParserState::ProcessingLoad;
  } else if (token.value == "STORE") {
    state = ParserState::ProcessingStore;
  } else if (token.value == "BEQ") {
    state = ParserState::ProcessingConditionalBranch;
  } else if (token.value == "CALL") {
    state = ParserState::ProcessingCall;
  } else if (token.value == "RET") {
    instructions.push_back(RetInstruction{});
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
    // throw logic_error(
        // format("Undefined behaviour: TokenType is Instruction but there is "
              //  "no code for handling instructions of type {}",
              //  token.value));
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
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = LoadInstruction{};
      get<LoadInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} with size {} is not a valid register name at PC {}",
                //  token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Offset) {
      // throw logic_error(
          // format("Expecting offset at {} but found {}", pc, token.value));
    }
    offset = stoi(token.value);
    if (offset > -17 && offset < 16) {
      get<LoadInstruction>(current_instruction).offset = offset;
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} is not a valid offset at PC {}", offset, pc));
    }
    break;
  case 2:
    if (token.type != TokenType::RegisterInOffset) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<LoadInstruction>(current_instruction).src_reg =
          token.value.at(1) - '0';
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
          // format("{} is not a valid register name at PC {}", token.value, pc));
    }
    break;
  }
}

void handle_store_instruction(vector<Instruction> &instructions,
                              Instruction &current_instruction,
                              ParserState &state, Address &pc,
                              uint8_t &argument_counter, int32_t &offset,
                              const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = StoreInstruction{};
      get<StoreInstruction>(current_instruction).src_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} with size {} is not a valid register name at PC {}",
                //  token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Offset) {
      // throw logic_error(
          // format("Expecting offset at {} but found {}", pc, token.value));
    }
    offset = stoi(token.value);
    if (offset > -17 && offset < 16) {
      get<StoreInstruction>(current_instruction).offset = offset;
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} is not a valid offset at PC {}", offset, pc));
    }
    break;
  case 2:
    if (token.type != TokenType::RegisterInOffset) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<StoreInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
          // format("{} is not a valid register name at PC {}", token.value, pc));
    }
    break;
  }
}

void handle_conditional_branch_instruction(vector<Instruction> &instructions,
                                           Instruction &current_instruction,
                                           ParserState &state, Address &pc,
                                           uint8_t &argument_counter,
                                           int32_t &offset,
                                           const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = ConditionalBranchInstruction{};
      get<ConditionalBranchInstruction>(current_instruction).src_reg1 =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} with size {} is not a valid register name at PC {}",
                //  token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Register) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<ConditionalBranchInstruction>(current_instruction).src_reg2 =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} with size {} is not a valid register name at PC {}",
                //  token.value, token.value.size(), pc));
    }
    break;
  case 2:
    if (token.type != TokenType::Number) {
      // throw logic_error(
          // format("Expecting offset at {} but found {}", pc, token.value));
    }
    offset = stoi(token.value);
    if (offset > -17 && offset < 16) {
      get<ConditionalBranchInstruction>(current_instruction).offset = offset;
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
          // format("{} is not a valid offset at PC {}", offset, pc));
    }
    break;
  }
}

void handle_call_instruction(vector<Instruction> &instructions,
                             Instruction &current_instruction,
                             ParserState &state, Address &pc,
                             uint8_t &argument_counter, int32_t &offset,
                             const Token &token) {
  if (token.type != TokenType::Identifier) {
    // throw logic_error(format("Expecting identifier/label at {} but found {}",
                            //  pc, token.value));
  }
  current_instruction = CallInstruction{};
  get<CallInstruction>(current_instruction).label = token.value;
  pc += 2;
  instructions.push_back(current_instruction);
  state = ParserState::ExpectingInstructionOrLabel;
}

void handle_add_instruction(vector<Instruction> &instructions,
                            Instruction &current_instruction,
                            ParserState &state, Address &pc,
                            uint8_t &argument_counter, int32_t &offset,
                            const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = AddInstruction{};
      get<AddInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
          // format("{} with size {} is not a valid register name at PC {}",
          //        token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Register) {
      // throw logic_error(
          // format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<AddInstruction>(current_instruction).src_reg1 =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 2:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<AddInstruction>(current_instruction).src_reg2 =
          token.value.at(1) - '0';
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  }
}

void handle_add_immediate_instruction(vector<Instruction> &instructions,
                                      Instruction &current_instruction,
                                      ParserState &state, Address &pc,
                                      uint8_t &argument_counter,
                                      int32_t &offset, const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = AddImmInstruction{};
      get<AddImmInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<AddImmInstruction>(current_instruction).src_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 2:
    if (token.type != TokenType::Number) {
      // throw logic_error(
      //     format("Expecting number at {} but found {}", pc, token.value));
    }
    offset = stoi(token.value);
    if (offset > -17 && offset < 16) {
      get<AddImmInstruction>(current_instruction).immediate = offset;
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
      //     format("{} is not a valid number at PC {}", offset, pc));
    }
    break;
  }
}

void handle_nand_instruction(vector<Instruction> &instructions,
                             Instruction &current_instruction,
                             ParserState &state, Address &pc,
                             uint8_t &argument_counter, int32_t &offset,
                             const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = NandInstruction{};
      get<NandInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<NandInstruction>(current_instruction).src_reg1 =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 2:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<NandInstruction>(current_instruction).src_reg2 =
          token.value.at(1) - '0';
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  }
}

void handle_mul_instruction(vector<Instruction> &instructions,
                            Instruction &current_instruction,
                            ParserState &state, Address &pc,
                            uint8_t &argument_counter, int32_t &offset,
                            const Token &token) {
  switch (argument_counter) {
  case 0:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      current_instruction = MulInstruction{};
      get<MulInstruction>(current_instruction).dest_reg =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 1:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<MulInstruction>(current_instruction).src_reg1 =
          token.value.at(1) - '0';
      argument_counter += 1;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  case 2:
    if (token.type != TokenType::Register) {
      // throw logic_error(
      //     format("Expecting register at {} but found {}", pc, token.value));
    }
    if (token.value.at(1) < '8' && token.value.at(1) >= '0' &&
        token.value.size() == 2) {
      get<MulInstruction>(current_instruction).src_reg2 =
          token.value.at(1) - '0';
      argument_counter = 0;
      pc += 2;
      instructions.push_back(current_instruction);
      state = ParserState::ExpectingInstructionOrLabel;
    } else {
      // throw logic_error(
      //     format("{} with size {} is not a valid register name at PC {}",
      //            token.value, token.value.size(), pc));
    }
    break;
  }
}
} // namespace parser_internals

ParserResult parse(const vector<Token> &tokens) {
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
      parser_internals::handle_instruction_type(state, pc, token, labels,
                                                instructions);
      break;
    case ParserState::ProcessingLoad:
      parser_internals::handle_load_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingStore:
      parser_internals::handle_store_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingConditionalBranch:
      parser_internals::handle_conditional_branch_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingCall:
      parser_internals::handle_call_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingAdd:
      parser_internals::handle_add_instruction(instructions,
                                               current_instruction, state, pc,
                                               argument_counter, offset, token);
      break;
    case ParserState::ProcessingAddImm:
      parser_internals::handle_add_immediate_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingNand:
      parser_internals::handle_nand_instruction(
          instructions, current_instruction, state, pc, argument_counter,
          offset, token);
      break;
    case ParserState::ProcessingMul:
      parser_internals::handle_mul_instruction(instructions,
                                               current_instruction, state, pc,
                                               argument_counter, offset, token);
      break;
    }
  }

  if (state != ParserState::ExpectingInstructionOrLabel) {
    // throw logic_error(
    //     format("The instruction at PC {} was not filled completely", pc));
  }

  return {instructions, labels, pc};
};

struct InstructionPrinter {
  void operator()(const LoadInstruction &inst) const {
    cout << "LOAD r" << inst.dest_reg << ", " << inst.offset << "(r"
         << inst.src_reg << ")" << endl;
  }
  void operator()(const StoreInstruction &inst) const {
    cout << "STORE r" << inst.src_reg << ", " << inst.offset << "(r"
         << inst.dest_reg << ")" << endl;
  }
  void operator()(const ConditionalBranchInstruction &inst) const {
    cout << "BEQ r" << inst.src_reg1 << ", r" << inst.src_reg2 << ", "
         << inst.offset << endl;
  }
  void operator()(const CallInstruction &inst) const {
    cout << "CALL " << inst.label << endl;
  }
  void operator()(const RetInstruction &) const { cout << "RET" << endl; }
  void operator()(const AddInstruction &inst) const {
    cout << "ADD r" << inst.dest_reg << ", r" << inst.src_reg1 << ", r"
         << inst.src_reg2 << endl;
  }
  void operator()(const AddImmInstruction &inst) const {
    cout << "ADDI r" << inst.dest_reg << ", r" << inst.src_reg << ", "
         << inst.immediate << endl;
  }
  void operator()(const NandInstruction &inst) const {
    cout << "NAND r" << inst.dest_reg << ", r" << inst.src_reg1 << ", r"
         << inst.src_reg2 << endl;
  }
  void operator()(const MulInstruction &inst) const {
    cout << "MUL r" << inst.dest_reg << ", r" << inst.src_reg1 << ", r"
         << inst.src_reg2 << endl;
  }
};

void print_instruction(const Instruction &instr) {
  visit(InstructionPrinter{}, instr);
}
