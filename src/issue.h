#ifndef ISSUE_H
#define ISSUE_H

#include <vector>
#include "Instruction.h"
#include "ReservationStation.h"

void issue(std::vector<Instruction> &instructions, std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations);

#endif // ISSUE_H
