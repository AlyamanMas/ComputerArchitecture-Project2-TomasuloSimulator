#include "Instruction.hpp"
#include "Processor.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

using namespace std;

enum class Kind {
  Load,
  Store,
  Beq,
  CallRet,
  AddAddi,
  NAND,
  Mul,
};

int main() {
  // string prog;
  // stringstream prog_stream;
  // prog_stream << cin.rdbuf();
  // prog = prog_stream.str();
  // vector<Token> tokens = tokenize(prog);
  // print_tokens(tokens);
  // auto [instructions, labels, pc] = parse(tokens);
  // for (auto i : instructions) {
  //   print_instruction(i);
  // }
  // cout << "PC is: " << pc << endl;

  Processor proc;

  std::vector<Instruction> instructions = {
      LoadInstruction{0, 1, 10, false, false, false},
      StoreInstruction{1, 2, 5, false, false, false},
      AddInstruction{0, 1, 2, false, false, false}};
  Address address;
  std::vector<ReservationStation<>> reservation_stations;

  int16_t a = 0;
  // Example ReservationStations
  ReservationStation<> rs1(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 5, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x1000, /* operation */ 0xAB, /* busy */ false,
      LoadInstruction{0, 1, 10, false, false, false});

  ReservationStation<> rs2(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 3, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x2000, /* operation */ 0xCD,
      /* busy */ false, StoreInstruction{1, 2, 5, false, false, false});

  ReservationStation<> rs3(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 4, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x3000, /* operation */ 0xEF,
      /* busy */ false, AddInstruction{0, 1, 2, false, false, false});

  // Add ReservationStations to vector
  reservation_stations.push_back(rs1);
  reservation_stations.push_back(rs2);
  reservation_stations.push_back(rs3);
  proc.processor(instructions, reservation_stations);
}
