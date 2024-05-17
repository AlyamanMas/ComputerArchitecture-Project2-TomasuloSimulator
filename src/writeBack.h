#ifndef WRITEBACK_H
#define WRITEBACK_H

#include <vector>
#include "Instruction.h"
#include "ReservationStation.h"

void writeback(std::vector<Instruction> &instructions, std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations);

#endif // WRITEBACK_H
