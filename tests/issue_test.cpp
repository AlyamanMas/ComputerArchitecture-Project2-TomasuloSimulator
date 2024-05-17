#include <gtest/gtest.h>
#include "../src/issue.h"

// Mock instruction types for testing
struct MockInstruction {
    bool issue = false;
    bool execute = false;
    bool write_result = false;
};

// Define your mock ReservationStation class for testing
class MockReservationStation : public ReservationStation<WordSigned, RSIndex> {
public:
    // Override getType() method to return the type of reservation station
    size_t getType() const override {
        // Return the type of reservation station based on your implementation
        return 0; // Example: Assuming type 0 for this mock
    }
};

// Test case for the issue function
TEST(ExecutionTest, IssueTest) {
    // Create a vector of mock instructions
    std::vector<Instruction> instructions = {
        MockInstruction(), MockInstruction(), MockInstruction()
    };

    // Create a vector of mock reservation stations
    std::vector<MockReservationStation> reservation_stations = {
        MockReservationStation(), MockReservationStation(), MockReservationStation()
    };

    // Call the issue function
    issue(instructions, reservation_stations);

    // Assert that all instructions are issued
    for (const auto &instr : instructions) {
        ASSERT_TRUE(std::visit([](const auto& i) { return i.issue; }, instr));
    }
}

// Entry point for the unit tests
int main(int argc, char **argv) {
    // Initialize the Google Test framework
    ::testing::InitGoogleTest(&argc, argv);
    // Run tests
    return RUN_ALL_TESTS();
}
