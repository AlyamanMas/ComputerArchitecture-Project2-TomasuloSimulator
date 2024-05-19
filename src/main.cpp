#include "Instruction.hpp"
#include "Processor.hpp"

#include <array>
#include <fstream>
#include <string>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <variant>
#include <vector>

using namespace std;

void run_test_case(const std::string &filename,
                   std::vector<Instruction> instructions,
                   std::vector<ReservationStation<>> reservation_stations) {
  std::ofstream out(filename);
  std::streambuf *coutbuf = std::cout.rdbuf(); // save old buffer
  std::cout.rdbuf(out.rdbuf());                // redirect std::cout to out

  Processor proc = Processor();
  proc.processor(instructions, reservation_stations);
  std::cout.rdbuf(coutbuf); // reset to standard output
}

int main() {
  // Define test cases
  std::vector<
      std::pair<std::string, std::pair<std::vector<Instruction>,
                                       std::vector<ReservationStation<>>>>>
      test_cases;

  // Test Case 1
  test_cases.push_back(
      {"test_case_1.txt",
       {{LoadInstruction{2, 1, 5, false, false, false},
         StoreInstruction{7, 7, 0, false, false, false},
         AddInstruction{1, 1, 1, false, false, false},
         AddInstruction{1, 1, 1, false, false, false},
         MulInstruction{1, 1, 1, false, false, false}},
        {ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 1,
                              /* kind */ ReservationStation<>::Kind::Load,
                              /* address */ 0x0000, /* operation */ 0xAB,
                              /* busy */ false, "Load"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 1,
                              /* kind */ ReservationStation<>::Kind::Store,
                              /* address */ 0x0001, /* operation */ 0xCD,
                              /* busy */ false, "Store"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 3,
                              /* kind */ ReservationStation<>::Kind::AddAddi,
                              /* address */ 0x0002, /* operation */ 0xEF,
                              /* busy */ false, "Add"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 3,
                              /* kind */ ReservationStation<>::Kind::AddAddi,
                              /* address */ 0x0002, /* operation */ 0xEF,
                              /* busy */ false, "Add"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 1,
                              /* kind */ ReservationStation<>::Kind::Mul,
                              /* address */ 0x0001, /* operation */ 0xCD,
                              /* busy */ false, "MUL")}}});

  // Test Case 2
  test_cases.push_back(
      {"test_case_2.txt",
       {{LoadInstruction{2, 1, 10, false, false, false},
         StoreInstruction{6, 7, 5, false, false, false},
         AddInstruction{0, 1, 2, false, false, false},
         AddInstruction{0, 1, 2, false, false, false}},
        {ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 2,
                              /* kind */ ReservationStation<>::Kind::Store,
                              /* address */ 0x0003, /* operation */ 0xAA,
                              /* busy */ false, "Store"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 2,
                              /* kind */ ReservationStation<>::Kind::Load,
                              /* address */ 0x0004, /* operation */ 0xBB,
                              /* busy */ false, "Load"),
         ReservationStation<>(/* j */ size_t(1), /* k */ size_t(1),
                              /* cycles_counter */ -1, /* cycles_for_exec */ 2,
                              /* kind */ ReservationStation<>::Kind::AddAddi,
                              /* address */ 0x0005, /* operation */ 0xCC,
                              /* busy */ false, "Add")}}});

  // Run all test cases
  for (const auto &test_case : test_cases) {
    run_test_case(test_case.first, test_case.second.first,
                  test_case.second.second);
  }

  return 0;
}
