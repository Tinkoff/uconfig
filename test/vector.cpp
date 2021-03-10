#include "uconfig/Objects.h"
#include "gtest/gtest.h"

TEST(Vector, NotInitialized)
{
    const auto vec = uconfig::Vector<int>();
    ASSERT_FALSE(vec.Initialized());
    ASSERT_FALSE(vec.Optional());

    ASSERT_THROW(vec.Get(), std::runtime_error);
    ASSERT_THROW(*vec, std::runtime_error);
    ASSERT_THROW((std::vector<int>)vec, std::runtime_error);
}

TEST(Vector, OptNotInitialized)
{
    const auto vec = uconfig::Vector<int>(true);
    ASSERT_FALSE(vec.Initialized());
    ASSERT_TRUE(vec.Optional());

    ASSERT_THROW(vec.Get(), std::runtime_error);
    ASSERT_THROW(*vec, std::runtime_error);
    ASSERT_THROW((std::vector<int>)vec, std::runtime_error);
}

TEST(Vector, Initialized)
{
    const auto vec = uconfig::Vector<int>({1, 2, 3});
    ASSERT_TRUE(vec.Initialized());
    ASSERT_TRUE(vec.Optional());
}

TEST(Vector, DirectCompare)
{
    const auto vec = uconfig::Vector<int>({1, 2, 3});

    ASSERT_TRUE(vec == uconfig::Vector<int>({1, 2, 3}));
    ASSERT_TRUE(vec != uconfig::Vector<int>({5, 6, 3, 4, 5, 4, 5}));
    ASSERT_TRUE(vec != uconfig::Vector<int>());
    ASSERT_TRUE(vec != uconfig::Vector<int>(true));
}

TEST(Vector, InDirectCompare)
{
    const auto test_vector = std::vector<int>{1, 2, 3};
    const auto vec = uconfig::Vector<int>(test_vector);

    ASSERT_TRUE(vec == test_vector);
    ASSERT_EQ(vec.Get(), test_vector);
    ASSERT_EQ(*vec, test_vector);
}

TEST(Vector, OperatorSquareBrackets)
{
    const auto vec = uconfig::Vector<int>({1, 2, 3});

    ASSERT_EQ(vec[0], 1);
    ASSERT_EQ(vec[1], 2);
    ASSERT_EQ(vec[2], 3);
}

TEST(Vector, Dereference)
{
    const auto vec = uconfig::Vector<int>({1, 2, 3});

    ASSERT_FALSE(vec->empty());
    ASSERT_EQ(vec->size(), 3);
    ASSERT_EQ(vec->at(0), 1);
    ASSERT_EQ(vec->front(), 1);
    ASSERT_EQ(vec->back(), 3);
}

TEST(Vector, RangeLoop)
{
    const auto vec = uconfig::Vector<int>({1, 2, 3});

    std::size_t pos = 0;
    for (const auto& elem : *vec) {
        ASSERT_EQ(elem, vec[pos++]);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
