#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "lpinterface.hpp"

using ::testing::AtLeast;

class MockLpSolver : public LinearProgramSolver {
  public:
    MOCK_METHOD0(solve, int());
};

TEST(MockingTest, Solve) {
  MockLpSolver lp;
  EXPECT_CALL(lp, solve()).Times(AtLeast(1));

  lp.solve();
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

