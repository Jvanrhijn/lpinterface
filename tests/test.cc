#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_lpsolver.hpp"
#include "mock_lp.hpp"

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

