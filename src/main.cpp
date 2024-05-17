#include "Instruction.hpp"
#include "Processor.hpp"

#include <iostream>
#include <sstream>
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
        LoadInstruction{0, 1, 10, false, false, false},
        StoreInstruction{1, 2, 5, false, false, false},
        AddInstruction{0, 1, 2, false, false, false}
    };

    Address address;
    // Define values for the constructor parameters
  // Assuming Value, RSIndex, Address, and Instruction are defined elsewhere
  ReservationStation rs1;
  rs1.cycles_counter = 0;
  rs1.cycles_for_exec = 0;
  rs1.kind = Kind Load;
  rs1.operation = 0;
  rs1.busy = false;

  // Create a vector of ReservationStation objects
  std::vector<ReservationStation> reservation_stations;

  // Add three ReservationStation objects to the vector
  reservation_stations.emplace_back(rs1);
  reservation_stations.emplace_back(rs1);
  reservation_stations.emplace_back(rs1);

    proc.processor(instructions, proc.reservation_stations);
}
