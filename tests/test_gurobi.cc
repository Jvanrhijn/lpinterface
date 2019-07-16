#include <gtest/gtest.h>
#include "lpinterface_gurobi.hpp"
#include "lp_impl.hpp"

using namespace lpint;

GurobiSolver create_grb() {
    LinearProgram lp(OptimizationType::Maximize, SparseMatrixType::RowWise);
    std::vector<Row<double>> rows =  {
        Row<double>({1.0, 1.0, 2.0}, {0, 1, 2}),
        Row<double>({1.0, 2.0, 3.0}, {0, 1, 2}),
        Row<double>({1,0, 1.0}, {0, 1})
    };
    lp.add_rows(rows);
    GurobiSolver grb(std::make_shared<LinearProgram>(lp));
    return grb;
}

TEST(Gurobi, SetParameters) {
    auto grb = create_grb();
    auto err = grb.set_parameter(Param::GrbThreads, 1);
    ASSERT_TRUE(err);
    err = grb.set_parameter(Param::GrbOutputFlag, 0);
    ASSERT_TRUE(err);
}