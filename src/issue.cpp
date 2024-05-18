#include "Issue.h"
#include <variant>

void Processor::issue(std::vector<Instruction> &instructions, std::vector<ReservationStation<WordSigned, RSIndex>> &reservation_stations) {
    for (auto &instr : instructions) {
        if (std::visit([](const auto &i) { return i.issue; }, instr)) {
            continue; 
        }

        bool foundStation = false;
        for (auto &station : reservation_stations) {
            if (!station.busy && station.operation == instr.index()) {
                std::visit([&station](auto &&i) {
                    station.busy = true;
                    i.issue = true;
                }, instr);
                foundStation = true;
                break;
            }
        }

        if (!foundStation) {
            break; 
        }
    }
}
