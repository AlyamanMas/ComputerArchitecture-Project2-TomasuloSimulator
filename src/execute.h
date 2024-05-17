#ifndef EXECUTION_H
#define EXECUTION_H

#include <vector>
#include "Instruction.h"
#include "ReservationStation.h"

void execute(std::vector<Instruction> &instructions, std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations);

#endif // EXECUTION_H
