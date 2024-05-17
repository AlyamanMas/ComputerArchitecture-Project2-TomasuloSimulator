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