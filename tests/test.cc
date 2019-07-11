#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "mock_lp.hpp"
#include "mock_lpsolver.hpp"

using ::testing::AtLeast;
using namespace lpint;

TEST(MockingTest, GetSolution) {
  MockLpSolver lp;
  EXPECT_CALL(lp, get_solution()).Times(AtLeast(1));

  lp.get_solution();
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
