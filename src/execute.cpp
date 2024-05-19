#include "execute.h"
#include <variant>

void Processor::execute(std::vector<Instruction> &instructions,
                        std::vector<ReservationStation<WordSigned, RSIndex>>
                            &reservation_stations) {
  for (auto &rs : reservation_stations) {
    // Check if the reservation station is busy and has finished the execution
    // cycles
    if (rs.busy && rs.cycles_counter == rs.cycles_for_exec) {
      // Perform the execution based on the type of instruction
      std::visit(
          [&rs](auto &instr) {
            using T = std::decay_t<decltype(instr)>;
            if constexpr (std::is_same_v<T, LoadInstruction>) {
              // Execute load instruction
              // For demonstration, assuming load execution involves fetching
              // data Replace with actual implementation
              instr.execute = true;
            } else if constexpr (std::is_same_v<T, StoreInstruction>) {
              // Execute store instruction
              instr.execute = true;
            } else if constexpr (std::is_same_v<T,
                                                ConditionalBranchInstruction>) {
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
          },
          rs.instr);

      // Update the reservation station to indicate it's ready for writeback
      rs.busy = false; // This would typically be done in the writeback phase,
                       // but we mark it here for simplicity
    } else if (rs.busy) {
      // Decrement the cycles counter if still executing
      rs.cycles_counter++;
    }
  }
}
