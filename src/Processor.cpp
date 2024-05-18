#include "Processor.hpp"
#include <chrono>
#include <functional> // Add this line for std::ref
#include <thread>
#include <type_traits> // For std::is_same_v
#include <variant>

using namespace std;
using WordSigned = int16_t;
using Value = int16_t;
// Index of reservation station producing the value. Here we use std::size_t
// to allow the user to use any number of reservation stations that can
// possibly be supported by the host hardware
using RSIndex = std::size_t;
struct RSIndices {
  RSIndex load, store, beq, call_ret, add_addi, nand, mul;
};

// Helper functions to the main three function in the simulator
bool Processor::isFinished(const std::vector<Instruction> &instructions) {
  for (const auto &instr : instructions) {
    if (std::visit([](const auto &i) { return !i.issue || !i.execute || !i.write_result; },
                   instr)) {
      return false;
    }
  }
  return true;
}

template <typename... Ts>
constexpr std::size_t get_variant_index(const std::variant<Ts...> &v) {
  std::size_t index = std::variant_npos; // Initialize with npos
  std::visit(
      [&index](const auto &arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::disjunction_v<std::is_same<T, Ts>...>) {
          std::size_t count = 0;
          ((std::is_same_v<T, Ts> ? index = count : ++count),
           ...); // Increment count for each alternative
        }
      },
      v);
  return index;
}

template <typename T>
bool Processor::areOperandsReady(T &instr, Processor &processor) {
  if constexpr (std::is_same_v<T, LoadInstruction>) {
    // Check operands for LoadInstruction
    return !processor.register_status_table[instr.src_reg].has_value();
  } else if constexpr (std::is_same_v<T, StoreInstruction>) {
    // Check operands for StoreInstruction
    return !processor.register_status_table[instr.src_reg].has_value() &&
           !processor.register_status_table[instr.dest_reg].has_value();
  } else if constexpr (std::is_same_v<T, ConditionalBranchInstruction>) {
    // Check operands for ConditionalBranchInstruction
    return !processor.register_status_table[instr.src_reg1].has_value() &&
           !processor.register_status_table[instr.src_reg2].has_value();
  } else if constexpr (std::is_same_v<T, CallInstruction>) {
    // Check operands for CallInstruction (no operands)
    return true;
  } else if constexpr (std::is_same_v<T, RetInstruction>) {
    // Check operands for RetInstruction (no operands)
    return true;
  } else if constexpr (std::is_same_v<T, AddInstruction>) {
    // Check operands for AddInstruction
    return !processor.register_status_table[instr.src_reg1].has_value() &&
           !processor.register_status_table[instr.src_reg2].has_value();
  } else if constexpr (std::is_same_v<T, AddImmInstruction>) {
    // Check operands for AddImmInstruction
    return !processor.register_status_table[instr.src_reg].has_value();
  } else if constexpr (std::is_same_v<T, NandInstruction>) {
    // Check operands for NandInstruction
    return !processor.register_status_table[instr.src_reg1].has_value() &&
           !processor.register_status_table[instr.src_reg2].has_value();
  } else if constexpr (std::is_same_v<T, MulInstruction>) {
    // Check operands for MulInstruction
    return !processor.register_status_table[instr.src_reg1].has_value() &&
           !processor.register_status_table[instr.src_reg2].has_value();
  } else {
    // Unsupported instruction type
    static_assert(std::is_same_v<T, T>, "Unsupported instruction type");
    return false;
  }
}

template <typename T>
std::pair<std::variant<Value, RSIndex>, std::variant<Value, RSIndex>>
Processor::getOperands(T &instr) {
  if constexpr (std::is_same_v<T, LoadInstruction>) {

    auto src1 = register_status_table[instr.src_reg].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg]);
    return {src1, {}};
  } else if constexpr (std::is_same_v<T, StoreInstruction>) {
    auto src1 = register_status_table[instr.src_reg].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg]);
    auto src2 = register_status_table[instr.dest_reg].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.dest_reg].value())
                    : std::variant<Value, RSIndex>(registers[instr.dest_reg]);
    return {src1, src2};
  } else if constexpr (std::is_same_v<T, ConditionalBranchInstruction>) {
    auto src1 = register_status_table[instr.src_reg1].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg1].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg1]);
    auto src2 = register_status_table[instr.src_reg2].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg2].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg2]);
    return {src1, src2};
  } else if constexpr (std::is_same_v<T, AddInstruction>) {
    auto src1 = register_status_table[instr.src_reg1].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg1].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg1]);
    auto src2 = register_status_table[instr.src_reg2].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg2].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg2]);
    return {src1, src2};
  } else if constexpr (std::is_same_v<T, AddImmInstruction>) {
    auto src1 = register_status_table[instr.src_reg].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg]);
    return {src1, {}};
  } else if constexpr (std::is_same_v<T, NandInstruction>) {
    auto src1 = register_status_table[instr.src_reg1].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg1].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg1]);
    auto src2 = register_status_table[instr.src_reg2].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg2].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg2]);
    return {src1, src2};
  } else if constexpr (std::is_same_v<T, MulInstruction>) {
    auto src1 = register_status_table[instr.src_reg1].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg1].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg1]);
    auto src2 = register_status_table[instr.src_reg2].has_value()
                    ? std::variant<Value, RSIndex>(
                          register_status_table[instr.src_reg2].value())
                    : std::variant<Value, RSIndex>(registers[instr.src_reg2]);
    return {src1, src2};
  } else {
    static_assert(std::is_same_v<T, T>, "Unsupported instruction type");
    return {{}, {}};
  }
}

std::optional<unsigned int>
Processor::getDestinationRegister(Instruction &instr) {
  return std::visit(
      [](const auto &instruction) -> std::optional<unsigned int> {
        if constexpr (std::is_same_v<decltype(instruction), LoadInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            StoreInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            ConditionalBranchInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            CallInstruction>) {
          // No destination register for call instruction
          return std::nullopt;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            RetInstruction>) {
          // No destination register for return instruction
          return std::nullopt;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            AddInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            AddImmInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            NandInstruction>) {
          return instruction.dest_reg;
        } else if constexpr (std::is_same_v<decltype(instruction),
                                            MulInstruction>) {
          return instruction.dest_reg;
        } else {
          // Handle unknown instruction type
          return std::nullopt;
        }
      },
      instr);
}

void Processor::printState(std::vector<Instruction> &instructions,
                           std::vector<ReservationStation<WordSigned, RSIndex>>
                               &reservation_stations) {
  std::cout << "Reservation Stations:" << std::endl;
  for (const auto &station : reservation_stations) {
    std::cout << "Unit Type: " << station.unit_type
              << ", Busy: " << station.busy << ", j: "
              << (station.j.index() == 0
                      ? std::to_string(std::get<Value>(station.j))
                      : "RS" + std::to_string(std::get<RSIndex>(station.j)))
              << ", k: "
              << (station.k.index() == 0
                      ? std::to_string(std::get<Value>(station.k))
                      : "RS" + std::to_string(std::get<RSIndex>(station.k)))
              << std::endl;
  }

  std::cout << "Instructions:" << std::endl;
  for (const auto &instr : instructions) {
    // Print instruction details
    std::visit(
        [&](const auto &instr) {
          std::cout << "Issue: " << instr.issue
                    << ", Execute: " << instr.execute
                    << ", Writeback: " << instr.write_result;
        },
        instr);
    std::cout << std::endl;
  }

  // std::cout << "Register Status Table:" << std::endl;
  // for (size_t i = 0; i < register_status_table.size(); ++i) {
  //   if (register_status_table[i].has_value()) {
  //     std::cout << "R" << i << ": RS" << register_status_table[i].value()
  //               << std::endl;
  //   } else {
  //     std::cout << "R" << i << ": Available" << std::endl;
  //   }
  // }
}

// Main three functions issue, execute, writeback
void Processor::issue(std::vector<Instruction> &instructions,
                      std::vector<ReservationStation<WordSigned, RSIndex>>
                          &reservation_stations) {
  bool instruction_found = false;
  int PC = 0;
  for (auto &instr : instructions) {
    if (std::visit([](const auto &i) { return i.issue; }, instr)) {
      PC++;
      continue;
    } else {

      bool issued = false;
      instruction_found = true;
      for (auto &station : reservation_stations) {
        std::visit(
            [&station, &issued, &PC, this](auto &&i) {
              if (!station.busy && station.unit_type == i.reservation_station &&
                  !issued && areOperandsReady(i, *this)) {
                station.busy = true;
                auto [op1, op2] = getOperands(i);
                station.j = op1;
                station.k = op2;
                station.address = PC;
                PC++;
                // station.instr = std::ref(i);
                i.issue = true;
                std::cout << "Issued instruction to " << i.reservation_station
                          << " station." << std::endl;
                issued = true;
              }
            },
            instr);
        if (issued)
          break;
      }
      if (instruction_found)
        break;
    }
  }
}

void Processor::execute(std::vector<Instruction> &instructions,
                        std::vector<ReservationStation<WordSigned, RSIndex>>
                            &reservation_stations) {
  for (auto &station : reservation_stations) {
    if (station.busy && !get_variant_index(station.j) &&
        !get_variant_index(station.k)) {

      std::visit(
          [&station, &instructions](auto &&instr) {
            if (!instr.execute) {
              instr.execution();
              station.cycles_counter++;
              if (station.cycles_counter >= station.cycles_for_exec) {
                station.busy = false;
                std::visit([](auto &&instr) { instr.execute = true; },
                           instructions[station.address]);
                station.cycles_counter = 0;
                std::cout << "Executed instruction in "
                          << instr.reservation_station << " station.--> "
                          << std::visit(
                                 [](auto &&instr) { return instr.execute; },
                                 instructions[station.address])
                          << std::endl;
              }
            }
          },
          instructions[station.address]);
    }
  }
}

void Processor::writeback(std::vector<Instruction> &instructions,
                          std::vector<ReservationStation<WordSigned, RSIndex>>
                              &reservation_stations) {
  for (auto &station : reservation_stations) {
    if (!station.busy && station.address != -1) {
      // Instruction& instr = station.instr;
      if (std::visit([](auto &&instr) { return instr.execute; },
                     instructions[station.address])) {
        // Get the destination register
        auto destRegOpt = getDestinationRegister(station.instr);
        if (destRegOpt.has_value()) {
          unsigned int destReg = destRegOpt.value();
          // Write back the result to the destination register
          register_status_table[destReg] = std::nullopt;
          std::cout << "Writeback: Result of instruction written to "
                       "destination register R"
                    << destReg << std::endl;
        }
        std::visit([](auto &&instr) { instr.write_result = true; },
                   instructions[station.address]); // Reset write_result flag
      }
    }
  }
}

int t = 0;

void Processor::processor(std::vector<Instruction> &instructions,
                          std::vector<ReservationStation<WordSigned, RSIndex>>
                              &reservation_stations) {
  cout << "cycle: " << t++ << endl;

  if (isFinished(instructions)) {
    return;
  } else {
    if (t < 10) {
      writeback(instructions, reservation_stations);
      execute(instructions, reservation_stations);
      issue(instructions, reservation_stations);
      printState(instructions, reservation_stations);
      processor(instructions, reservation_stations);
    } else {
      return;
    }
  }
}
