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

std::vector<int> readVectorFromFile(const std::string &filename) {
  std::vector<int> vec;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << std::endl;
    return vec;
  }

  int value;
  while (file >> value) {
    vec.push_back(value);
  }

  file.close();
  return vec;
}

void run_assembly_file(const std::string &filename,
                       vector<ReservationStation<>> &reservation_stations) {
  ifstream in(filename);
  stringstream instream;
  instream << in.rdbuf();
  string file_content = instream.str();

  auto tokens = tokenize(file_content);
  auto [instructions, labels, _pc] = parse(tokens);
  print_tokens(tokens, labels);
  auto processor = Processor();
  processor.processor(instructions, reservation_stations, labels);
}

std::vector<ReservationStation<>>
createReservationStations(const std::vector<int> &counts,
                          const std::vector<int> &cycles) {
  std::vector<ReservationStation<>> reservation_stations;

  int address = 0x0000;
  int cycle_index = 0;
  for (int i = 0; i < counts.size(); ++i) {
    for (int j = 0; j < counts[i]; ++j) {
      ReservationStation<>::Kind kind =
          static_cast<ReservationStation<>::Kind>(i);
      std::string type_name;
      switch (kind) {
      case ReservationStation<>::Kind::Load:
        type_name = "Load";
        break;
      case ReservationStation<>::Kind::Store:
        type_name = "Store";
        break;
      case ReservationStation<>::Kind::Beq:
        type_name = "BEQ";
        break;
      case ReservationStation<>::Kind::CallRet:
        type_name = "CallRet";
        break;
      case ReservationStation<>::Kind::AddAddi:
        type_name = "Add";
        break;
      case ReservationStation<>::Kind::NAND:
        type_name = "Nand";
        break;
      case ReservationStation<>::Kind::Mul:
        type_name = "MUL";
        break;
      default:
        type_name = "Unknown";
      }

      reservation_stations.push_back(
          ReservationStation<>(size_t(1),           // j
                               size_t(1),           // k
                               -1,                  // cycles_counter
                               cycles[cycle_index], // cycles_for_exec
                               kind,                // kind
                               0,                   // address
                               0,                   // operation
                               false,               // busy
                               type_name));         // unit type

      cycle_index++;
    }
  }

  return reservation_stations;
}

int main() {
  std::vector<int> counts = readVectorFromFile("./tests/counts.txt");
  std::vector<int> cycles = readVectorFromFile("./tests/cycles.txt");
  // Create reservation stations based on user-specified counts and cycles
  std::vector<ReservationStation<>> reservation_stations =
      createReservationStations(counts, cycles);

  // Now you can pass this reservation_stations vector to your run_assembly_file
  // function
  run_assembly_file("./tests/call_test.asm", reservation_stations);
  return 0;
}
