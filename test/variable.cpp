#include "uconfig/Objects.h"
#include "gtest/gtest.h"

TEST(Variable, NotInitialized)
{
    const auto var = uconfig::Variable<int>();
    ASSERT_FALSE(var.Initialized());
    ASSERT_FALSE(var.Optional());

    ASSERT_THROW(var.Get(), std::runtime_error);
    ASSERT_THROW(*var, std::runtime_error);
    ASSERT_THROW(var + 1, std::runtime_error);
    ASSERT_THROW(var - 1, std::runtime_error);
}

TEST(Variable, Initialized)
{
    const auto var = uconfig::Variable<int>(123);
    ASSERT_TRUE(var.Initialized());
    ASSERT_TRUE(var.Optional());
}

TEST(Variable, DirectCompare)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_TRUE(var == uconfig::Variable<int>(123));
    ASSERT_TRUE(var != uconfig::Variable<int>(5634545));
    ASSERT_TRUE(var != uconfig::Variable<int>());
}

TEST(Variable, InDirectCompare)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_TRUE(var == 123);
    ASSERT_EQ(var.Get(), 123);
    ASSERT_EQ(*var, 123);
}

TEST(Variable, OperatorPlus)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_EQ(var + 123, 246);
    ASSERT_EQ(var + 123, 123 + var);
}

TEST(Variable, OperatorMinus)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_EQ(var - 100, 23);
    ASSERT_EQ(var - 100, 146 - var);
}

TEST(Variable, OperatorMore)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_TRUE(var > 100);
    ASSERT_TRUE(var >= 100);
    ASSERT_TRUE(var >= 123);
    ASSERT_TRUE(200 > var);
    ASSERT_TRUE(200 >= var);
    ASSERT_TRUE(123 >= var);
}

TEST(Variable, OperatorLess)
{
    const auto var = uconfig::Variable<int>(123);

    ASSERT_TRUE(var < 200);
    ASSERT_TRUE(var <= 200);
    ASSERT_TRUE(var <= 123);
    ASSERT_TRUE(100 < var);
    ASSERT_TRUE(100 <= var);
    ASSERT_TRUE(123 <= var);
}

TEST(Variable, OperatorStream)
{
    std::ostringstream ss;
    ss << uconfig::Variable<int>(123);
    ASSERT_EQ(ss.str(), "123");

    ss.str("");
    ss << uconfig::Variable<int>();
    ASSERT_EQ(ss.str(), "[not set]");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
