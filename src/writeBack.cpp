#include "writeback.h"
#include <variant>

void writeback(std::vector<Instruction> &instructions, std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations) {
    for (auto &rs : reservation_stations) {
        if (!rs.busy && rs.cycles_counter == rs.cycles_for_exec) {
            std::visit([&rs, &instructions](auto &instr) {
                using T = std::decay_t<decltype(instr)>;
                if constexpr (std::is_same_v<T, LoadInstruction>) {
                    instructions[instr.pc].writeback = true;
                } else if constexpr (std::is_same_v<T, StoreInstruction>) {
                    // No writeback needed for store
                } else if constexpr (std::is_same_v<T, ConditionalBranchInstruction>) {
                    // No writeback needed for conditional branch
                } else if constexpr (std::is_same_v<T, CallInstruction>) {
                    // No writeback needed for call
                } else if constexpr (std::is_same_v<T, RetInstruction>) {
                    // No writeback needed for return
                } else if constexpr (std::is_same_v<T, AddInstruction>) {
                    instructions[instr.pc].writeback = true;
                } else if constexpr (std::is_same_v<T, AddImmInstruction>) {
                    instructions[instr.pc].writeback = true;
                } else if constexpr (std::is_same_v<T, NandInstruction>) {
                    instructions[instr.pc].writeback = true;
                } else if constexpr (std::is_same_v<T, MulInstruction>) {
                    instructions[instr.pc].writeback = true;
                }
            }, rs.instr);
        }
    }
}
