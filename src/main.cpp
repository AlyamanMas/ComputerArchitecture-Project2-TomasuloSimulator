#include "Instruction.hpp"
#include "Processor.hpp"

#include <array>
#include <string>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

using namespace std;

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
      LoadInstruction{2, 1, 10, false, false, false},
      StoreInstruction{2, 7, 5, false, false, false},
      AddInstruction{0, 2, 2, false, false, false},
      AddInstruction{0, 1, 2, false, false, false}};

      std::vector<Instruction> fake_instructions = {
      LoadInstruction{2, 1, 10, false, false, false},
      StoreInstruction{6, 7, 5, false, false, false},
      AddInstruction{0, 1, 2, false, false, false},
      AddInstruction{0, 1, 2, false, false, false}};
  Address address;
  std::vector<ReservationStation<>> reservation_stations;

  std::variant<int16_t, size_t> a = size_t(1);
  // Example ReservationStations
  ReservationStation<> rs1(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 1, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x0000, /* operation */ 0xAB, /* busy */ false,
      fake_instructions[0], "Load");

  ReservationStation<> rs2(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 1, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x0001, /* operation */ 0xCD,
      /* busy */ false, fake_instructions[1], "Store");

  ReservationStation<> rs3(
      /* j */ a, /* k */ a, /* cycles_counter */ 0,
      /* cycles_for_exec */ 1, /* kind */ ReservationStation<>::Kind::Store,
      /* address */ 0x0002, /* operation */ 0xEF,
      /* busy */ false, fake_instructions[2], "Add");

  // Add ReservationStations to vector
  reservation_stations.push_back(rs1);
  reservation_stations.push_back(rs2);
  reservation_stations.push_back(rs3);
  proc.processor(instructions, reservation_stations);
}
