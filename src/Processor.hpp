// #ifndef INCLUDE_SRC_PROCESSOR_HPP_
#include <map>
#define INCLUDE_SRC_PROCESSOR_HPP_

#include "Instruction.hpp"

#include <array>
#include <cmath>
#include <string>

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

struct NoMsg {};

struct BranchMsg {
  Address to;
};

template <typename Value = int16_t> struct UpdateRegMsg {
  unsigned int register_idx : 3;
  Value value;
};

using RSMsg = std::variant<BranchMsg, UpdateRegMsg<>, NoMsg>;

// Since we can have one reservation station be able to do multiple operations,
// we need to store the operations in the reservation station. The following
// enum helps with standardizing them.
enum Operations {
  // Null/Uninitialized operation, when reservation station is not busy. Also
  // could be used for reservation stations that only have one operation
  // per kind
  Null,
  Call,
  Ret,
  Add,
  Addi,
};

template <typename Value = int16_t, typename RSIndex = std::size_t>
class ReservationStation {
public:
  enum class Kind {
    Load,
    Store,
    Beq,
    CallRet,
    AddAddi,
    NAND,
    Mul,
  };

  ReservationStation(std::variant<Value, RSIndex> j,
                     std::variant<Value, RSIndex> k, size_t cycles_counter,
                     size_t cycles_for_exec, Kind kind, Address address,
                     uint8_t operation, bool busy, const std::string unit_type)
      : j(std::move(j)), k(std::move(k)), cycles_counter(cycles_counter),
        cycles_for_exec(cycles_for_exec), kind(kind), address(address),
        operation(operation), busy(busy), unit_type(unit_type) {}

  RSMsg do_cycle();

  // Since "Only V or Q is set for a source (never both) (lec16, page 7)", we
  // use std::variant to statically ensure this property is preserved using the
  // type system
  std::variant<Value, RSIndex> j, k;
  size_t cycles_counter, cycles_for_exec;
  Kind kind;
  // This is used to track the execution/address computation
  Address address;
  uint8_t operation;
  bool busy;
  std::string unit_type;
};

class Processor {
public:
  using WordSigned = int16_t;
  // Index of reservation station producing the value. Here we use std::size_t
  // to allow the user to use any number of reservation stations that can
  // possibly be supported by the host hardware
  using RSIndex = std::size_t;
  using Value = int16_t;
  struct RSIndices {
    RSIndex load, store, beq, call_ret, add_addi, nand, mul;
  };

  // Since we can have a dynamic number of reservation stations, we need to be
  // able to keep track of where each type starts and ends so we can easily
  // access them.
  RSIndices rs_indices{};
  // The memory is word addressable as per the requirements of the project
  // just change the memory for testing
  std::array<WordSigned, 8> memory{};
  std::array<WordSigned, 8> registers{1, 1, 1, 1, 1, 1, 1, 1};

  // This is the table that tells us if a register is awaiting result from a
  // reservation station or not. The value is std::optional<RSIndex> such that
  // when we are waiting for a reservation station, the optional is engaged and
  // .has_value() is true; while when we are not waiting, the optional is
  // disengaged and .has_value() is false.
  std::array<std::optional<RSIndex>, 8> register_status_table{};
  std::vector<ReservationStation<WordSigned, RSIndex>> reservation_stations{};

  bool isFinished(const std::vector<Instruction> &instructions);
  template <typename T>
  std::pair<std::variant<Value, RSIndex>, std::variant<Value, RSIndex>>
  getOperands(T &instr);

  template <typename T> bool areOperandsReady(T &instr, Processor &processor);
  void printState(std::vector<Instruction> &instructions,
                  std::vector<ReservationStation<WordSigned, RSIndex>>
                      &reservation_stations);

  std::optional<unsigned int> getDestinationRegister(Instruction &instr);

  void issue(std::vector<Instruction> &instructions,
             std::vector<ReservationStation<WordSigned, RSIndex>>
                 &reservation_stations);
  void execute(std::vector<Instruction> &instructions,
               std::vector<ReservationStation<WordSigned, RSIndex>>
                   &reservation_stations,
               const std::map<std::string, Address> &labels);
  void writeback(std::vector<Instruction> &instructions,
                 std::vector<ReservationStation<WordSigned, RSIndex>>
                     &reservation_stations);

  void processor(std::vector<Instruction> &instructions,
                 std::vector<ReservationStation<WordSigned, RSIndex>>
                     &reservation_stations,
                 const std::map<std::string, Address> &labels);
};
