#include "Processor.hpp"
#include <chrono>
#include <functional> // Add this line for std::ref
#include <thread>
#include <type_traits> // For std::is_same_v
#include <variant>
int cycles = 0;
int PC_counter = 0;
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
    if (std::visit(
            [](const auto &i) {
              return !i.issue || !i.execute || !i.write_result;
            },
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
                    << ", Start_Execute: " << instr.start_execution
                    << ", Finish Execute: " << instr.execute
                    << ", Writeback: " << instr.write_result
                    << ", dest_reg: " << instr.dest_reg;
        },
        instr);
    std::cout << std::endl;
  }

  std::cout << "Register Status Table:" << std::endl;
  for (size_t i = 0; i < register_status_table.size(); ++i) {
    if (register_status_table[i].has_value()) {
      std::cout << "R" << i << ": RS" << register_status_table[i].value()
                << std::endl;
    } else {
      std::cout << "R" << i << ": Available" << std::endl;
    }
  }
  std::cout << "Register File:" << std::endl;
  for (size_t i = 0; i < registers.size(); ++i) {
    std::cout << "R" << i << ": " << registers[i] << std::endl;
  }

  std::cout << "Memory:" << std::endl;
  for (size_t i = 0; i < memory.size(); ++i) {
    std::cout << "Mem" << i << ": " << memory[i] << std::endl;
  }
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
                  !issued) {
                station.busy = true;
                auto [op1, op2] = getOperands(i);
                station.j = op1;
                station.k = op2;
                station.address = PC;
                station.cycles_counter =
                    -1; // Initialize cycle counter for new instruction
                i.issue = cycles;
                PC_counter++;

                // Update the register status table if the instruction has a
                // destination register
                if constexpr (std::is_same_v<std::decay_t<decltype(i)>,
                                             LoadInstruction> ||
                              std::is_same_v<std::decay_t<decltype(i)>,
                                             AddInstruction> ||
                              std::is_same_v<std::decay_t<decltype(i)>,
                                             AddImmInstruction> ||
                              std::is_same_v<std::decay_t<decltype(i)>,
                                             NandInstruction> ||
                              std::is_same_v<std::decay_t<decltype(i)>,
                                             MulInstruction>) {
                  register_status_table[i.dest_reg] = station.address;
                }

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
    if (station.busy && !std::holds_alternative<RSIndex>(station.j) &&
        !std::holds_alternative<RSIndex>(station.k)) {
      std::visit(
          [&station, &instructions, this, &PC_counter](auto &&instr) {
            if (!instr.execute) {
              // Execute the instruction
              instr.execution();
              if (!instr.start_execution) {
                instr.start_execution =
                    cycles + 1; // Track when the execution starts
              }
              station.cycles_counter++;

              if (station.cycles_counter >= station.cycles_for_exec) {
                station.busy = false;
                std::visit([](auto &&instr) { instr.execute = cycles; },
                           instructions[station.address]);

                // Determine if the result should be stored in a register or
                // memory
                if constexpr (std::is_same_v<std::decay_t<decltype(instr)>,
                                             LoadInstruction>) {
                  registers[instr.dest_reg] =
                      memory[instr.src_reg + instr.offset];
                  register_status_table[instr.dest_reg].reset();
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         StoreInstruction>) {
                  memory[instr.dest_reg + instr.offset] =
                      registers[instr.src_reg];
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         AddInstruction>) {
                  registers[instr.dest_reg] =
                      registers[instr.src_reg1] + registers[instr.src_reg2];
                  register_status_table[instr.dest_reg].reset();
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         AddImmInstruction>) {
                  registers[instr.dest_reg] =
                      registers[instr.src_reg] + instr.immediate;
                  register_status_table[instr.dest_reg].reset();
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         NandInstruction>) {
                  registers[instr.dest_reg] =
                      ~(registers[instr.src_reg1] & registers[instr.src_reg2]);
                  register_status_table[instr.dest_reg].reset();
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         MulInstruction>) {
                  registers[instr.dest_reg] =
                      registers[instr.src_reg1] * registers[instr.src_reg2];
                  register_status_table[instr.dest_reg].reset();
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         ConditionalBranchInstruction>) {
                    // Branch logic (simplified)
                    // Assume PC is part of the processor state
                    // PC += instr.offset;
                    bool branch_taken = (registers[instr.src_reg1] == registers[instr.src_reg2]);
                                if (branch_taken) {
                                    PC_counter = instr.offset + instr[station.address];  // Update PC based on branch prediction
                                    // Perform any additional actions needed when the branch is taken
                                    std::cout << "Branch taken. PC updated to " << PC_counter << std::endl;
                                } else {
                                    // Handle the branch not taken scenario if needed
                                }
                                // Handle branch prediction update here if needed
                  
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         CallInstruction>) {
                  // Call logic (simplified)
                  // Assume PC is part of the processor state
                  // PC = label_address;
                  PC_counter = instr.label;
                } else if constexpr (std::is_same_v<
                                         std::decay_t<decltype(instr)>,
                                         RetInstruction>) {
                  // Return logic (simplified)
                  // Assume PC is part of the processor state
                  // PC = return_address;
                  // PC_counter = instr.return_address;
                }

                station.cycles_counter = -1;
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
    if (!station.busy && station.address != static_cast<Address>(-1)) {
      // Check if the instruction at the station has executed but not yet
      // written back
      if (std::visit(
              [](auto &&instr) {
                return (instr.execute && !bool(instr.write_result));
              },
              instructions[station.address])) {

        // Retrieve the instruction result
        WordSigned result = std::visit(
            [](auto &&instr) -> WordSigned {
              if constexpr (std::is_same_v<std::decay_t<decltype(instr)>,
                                           LoadInstruction> ||
                            std::is_same_v<std::decay_t<decltype(instr)>,
                                           AddInstruction> ||
                            std::is_same_v<std::decay_t<decltype(instr)>,
                                           AddImmInstruction> ||
                            std::is_same_v<std::decay_t<decltype(instr)>,
                                           NandInstruction> ||
                            std::is_same_v<std::decay_t<decltype(instr)>,
                                           MulInstruction>) {
                return instr.result.value();
              } else {
                return WordSigned(); // Default return type, should not reach
                                     // here
              }
            },
            instructions[station.address]);

        // Update registers and reservation stations
        for (size_t i = 0; i < register_status_table.size(); ++i) {
          if (register_status_table[i] &&
              register_status_table[i] == station.address) {
            registers[i] = result;
            register_status_table[i] = std::nullopt;
            std::cout << "Writeback: Result written to register R" << i
                      << " with value " << result << std::endl;
          }
        }

        for (auto &res_station : reservation_stations) {
          if (std::holds_alternative<RSIndex>(res_station.j) &&
              std::get<RSIndex>(res_station.j) == station.address) {
            res_station.j = result;
          }
          if (std::holds_alternative<RSIndex>(res_station.k) &&
              std::get<RSIndex>(res_station.k) == station.address) {
            res_station.k = result;
          }
        }

        // Mark the instruction's write_result flag as true
        std::visit([](auto &&instr) { instr.write_result = cycles + 1; },
                   instructions[station.address]);

        // Clear the reservation station
        station.busy = false;
        station.address = static_cast<Address>(-1);
      }
    }
  }
}

void Processor::processor(std::vector<Instruction> &instructions,
                          std::vector<ReservationStation<WordSigned, RSIndex>>
                              &reservation_stations) {

  if (isFinished(instructions)) {
    cout << "IPC: " << float(instructions.size()) / float(cycles) << endl;
    cout << "Total Execution time: " << float(cycles) << endl;
    return;
  } else {
    if (cycles < 20) {
      cout << "cycle: " << cycles++ << endl;

      issue(instructions, reservation_stations);
      execute(instructions, reservation_stations);
      writeback(instructions, reservation_stations);
      printState(instructions, reservation_stations);
      processor(instructions, reservation_stations);
    } else {
      return;
    }
  }
}
