#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lpinterface.hpp"

using namespace lpint;

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
