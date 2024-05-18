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
bool Processor::isFinished(const std::vector<Instruction> &instructions) {
  for (const auto &instr : instructions) {
    if (std::visit([](const auto &i) { return !i.issue; }, instr)) {
      return false;
    }
  }
  return true;
}

void Processor::issue(std::vector<Instruction> &instructions,
                      std::vector<ReservationStation<WordSigned, RSIndex>>
                          &reservation_stations) {
  for (auto &instr : instructions) {
    if (std::visit([](const auto &i) { return i.issue; }, instr)) {
      continue;
    }

    bool issued = false;
    for (auto &station : reservation_stations) {
      std::visit(
          [&station, &issued](auto &&i) {
            if (!station.busy && station.unit_type == i.reservation_station &&
                !issued) {
              station.busy = true;
              i.issue = true;
              std::cout << "Issued instruction to " << i.reservation_station
                        << " station." << std::endl;
              issued = true;
            }
          },
          instr);

      if (issued) {
        break;
      }
    }

    if (issued) {
      break;
    }
  }
}

void Processor::writeback(std::vector<Instruction> &instructions,
                          std::vector<ReservationStation<WordSigned, RSIndex>>
                              &reservation_stations) {}
int t = 0;

void Processor::processor(std::vector<Instruction> &instructions,
                          std::vector<ReservationStation<WordSigned, RSIndex>>
                              &reservation_stations) {
  cout << "cycle: " << t++ << endl;

  if (isFinished(instructions)) {
    return;
  } else {
    issue(instructions, reservation_stations);
    processor(instructions, reservation_stations);

    // writeback(instructions, reservation_stations);
    // execute(instructions, reservation_stations);
  }
