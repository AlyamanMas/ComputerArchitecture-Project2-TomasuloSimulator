#include "Processor.hpp"
using namespace std;
using WordSigned = int16_t;
// Index of reservation station producing the value. Here we use std::size_t
// to allow the user to use any number of reservation stations that can
// possibly be supported by the host hardware
using RSIndex = std::size_t;
struct RSIndices {
  RSIndex load, store, beq, call_ret, add_addi, nand, mul;
};
bool isFinished(const std::vector<Instruction> &instructions) {
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

void issue(std::vector<Instruction> &instructions,
           std::vector<ReservationStation<WordSigned, RSIndex>>
               &reservation_stations) {
  // Iterate through instructions
  for (auto &instr : instructions) {
    // Check if the instruction is already issued
    if (std::visit([](const auto &i) { return i.issue; }, instr)) {
      continue; // Instruction already issued, skip
    }

    // Find a free reservation station of the same type as the instruction
    bool foundStation = false;
    for (auto &station : reservation_stations) {
      if (!station.busy && station.operation == instr.index()) {
        // Issue the instruction to the reservation station
        std::visit(
            [&station](auto &&i) {
                // Put it in the reservation station
                station.busy = 1;

              // Mark the instruction as issued
              i.issue = true;
            },
            instr);
        foundStation = true;
        break;
      }
    }

    if (!foundStation) {
      // No free reservation station available for this instruction type, cannot
      // issue instruction now
      break; // Stop issuing further instructions
    }
  }
}

void execute(std::vector<Instruction> &instructions,
             std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations) {

    for (auto &rs : reservation_stations) {
        // Check if the reservation station is busy and has finished the execution cycles
        if (rs.busy && rs.cycles_counter == rs.cycles_for_exec) {
            // Perform the execution based on the type of instruction
            std::visit([&rs](auto &instr) {
                using T = std::decay_t<decltype(instr)>;
                if constexpr (std::is_same_v<T, LoadInstruction>) {
                    // Execute load instruction
                    // For demonstration, assuming load execution involves fetching data
                    // Replace with actual implementation
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, StoreInstruction>) {
                    // Execute store instruction
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, ConditionalBranchInstruction>) {
                    // Execute conditional branch
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, CallInstruction>) {
                    // Execute call
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, RetInstruction>) {
                    // Execute return
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, AddInstruction>) {
                    // Execute addition
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, AddImmInstruction>) {
                    // Execute add immediate
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, NandInstruction>) {
                    // Execute NAND
                    instr.execute = true;
                } else if constexpr (std::is_same_v<T, MulInstruction>) {
                    // Execute multiplication
                    instr.execute = true;
                }
            },rs.instr );

            // Update the reservation station to indicate it's ready for writeback
            rs.busy = false; // This would typically be done in the writeback phase, but we mark it here for simplicity
        } else if (rs.busy) {
            // Decrement the cycles counter if still executing
            rs.cycles_counter++;
        }
    }
}
void writeback(std::vector<Instruction> &instructions,
               std::vector<ReservationStation<WordSigned, RSIndex>>
                   &reservation_stations) {}
int t = 0;
void processor(std::vector<Instruction> &instructions,
               std::vector<ReservationStation<WordSigned, RSIndex>>
                   &reservation_stations) {
  if (isFinished(instructions)) {
    return;
  }
  writeback(instructions, reservation_stations);
  execute(instructions, reservation_stations);
  issue(instructions, reservation_stations);
  cout << "cycle: " << t++;
}