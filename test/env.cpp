#include "uconfig/format/Env.h"
#include "gtest/gtest.h"

/* Since env values are just string we try to convert if possible */

static const std::map<std::string, std::string> env_source = {
    {"STRING", "value"},
    {"POSINTEGER", "123"},
    {"NEGINTEGER", "-123"},
    {"POSLONGINTEGER", "123456789000"},
    {"NEGLONGINTEGER", "-123456789000"},
    {"POSDOUBLE", "123456.789"},
    {"NEGDOUBLE", "-123456.789"},
};

void SetEnv()
{
    for (const auto& [name, value] : env_source) {
        setenv(name.c_str(), value.c_str(), 1);
    }
}

void ClearEnv()
{
    for (const auto& [name, _] : env_source) {
        unsetenv(name.c_str());
    }
}

template <typename T>
testing::AssertionResult Parsed(const std::string& name, const T& expected_value)
{
    std::optional<T> value = uconfig::EnvFormat{}.Parse<T>(nullptr, name);
    if (!value.has_value()) {
        return ::testing::AssertionFailure() << "'" << name << "' env variable was not parsed";
    }

    if (*value != expected_value) {
        return ::testing::AssertionFailure() << "'" << name << "' env variable value '" << *value
                                             << "' differs from expected '" << expected_value << "'";
    }

    return testing::AssertionSuccess();
}

template <typename T>
testing::AssertionResult NotParsed(const std::string& name)
{
    std::optional<T> value = uconfig::EnvFormat{}.Parse<T>(nullptr, name);
    if (value.has_value()) {
        return ::testing::AssertionFailure() << "'" << name << "' env variable was parsed";
    }

    return testing::AssertionSuccess();
}

TEST(Env, ParseNoValue)
{
    ClearEnv();

    ASSERT_TRUE(NotParsed<std::string>("STRING"));
    ASSERT_TRUE(NotParsed<int>("POSINTEGER"));
    ASSERT_TRUE(NotParsed<int>("NEGINTEGER"));
    ASSERT_TRUE(NotParsed<long int>("POSLONGINTEGER"));
    ASSERT_TRUE(NotParsed<long int>("NEGLONGINTEGER"));
    ASSERT_TRUE(NotParsed<double>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<double>("NEGDOUBLE"));
}

TEST(Env, ParseAsString)
{
    SetEnv();

    ASSERT_TRUE(Parsed<std::string>("STRING", "value"));
    ASSERT_TRUE(Parsed<std::string>("POSINTEGER", "123"));
    ASSERT_TRUE(Parsed<std::string>("NEGINTEGER", "-123"));
    ASSERT_TRUE(Parsed<std::string>("POSLONGINTEGER", "123456789000"));
    ASSERT_TRUE(Parsed<std::string>("NEGLONGINTEGER", "-123456789000"));
    ASSERT_TRUE(Parsed<std::string>("POSDOUBLE", "123456.789"));
    ASSERT_TRUE(Parsed<std::string>("NEGDOUBLE", "-123456.789"));
}

TEST(Env, ParseAsInt)
{
    SetEnv();

    ASSERT_TRUE(Parsed<int>("POSINTEGER", 123));
    ASSERT_TRUE(Parsed<int>("NEGINTEGER", -123));

    ASSERT_TRUE(NotParsed<int>("STRING"));
    ASSERT_TRUE(NotParsed<int>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<int>("NEGDOUBLE"));
    ASSERT_TRUE(NotParsed<int>("POSLONGINTEGER"));
    ASSERT_TRUE(NotParsed<int>("NEGLONGINTEGER"));
}

TEST(Env, ParseAsUnsignedInt)
{
    SetEnv();

    ASSERT_TRUE(Parsed<unsigned>("POSINTEGER", 123));

    ASSERT_TRUE(NotParsed<unsigned>("STRING"));
    ASSERT_TRUE(NotParsed<unsigned>("NEGINTEGER"));
    ASSERT_TRUE(NotParsed<unsigned>("POSLONGINTEGER"));
    ASSERT_TRUE(NotParsed<unsigned>("NEGLONGINTEGER"));
    ASSERT_TRUE(NotParsed<unsigned>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<unsigned>("NEGDOUBLE"));
}

TEST(Env, ParseAsLongInt)
{
    SetEnv();

    ASSERT_TRUE(Parsed<long int>("POSINTEGER", 123));
    ASSERT_TRUE(Parsed<long int>("NEGINTEGER", -123));
    ASSERT_TRUE(Parsed<long int>("POSLONGINTEGER", 123456789000));
    ASSERT_TRUE(Parsed<long int>("NEGLONGINTEGER", -123456789000));

    ASSERT_TRUE(NotParsed<long int>("STRING"));
    ASSERT_TRUE(NotParsed<long int>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<long int>("NEGDOUBLE"));
}

TEST(Env, ParseAsUnsignedLongInt)
{
    SetEnv();

    ASSERT_TRUE(Parsed<unsigned long int>("POSINTEGER", 123));
    ASSERT_TRUE(Parsed<unsigned long int>("POSLONGINTEGER", 123456789000));

    ASSERT_TRUE(NotParsed<unsigned long int>("STRING"));
    ASSERT_TRUE(NotParsed<unsigned long int>("NEGINTEGER"));
    ASSERT_TRUE(NotParsed<unsigned long int>("NEGLONGINTEGER"));
    ASSERT_TRUE(NotParsed<unsigned long int>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<unsigned long int>("NEGDOUBLE"));
}

TEST(Env, ParseAsDouble)
{
    SetEnv();

    ASSERT_TRUE(Parsed<double>("POSINTEGER", 123.0));
    ASSERT_TRUE(Parsed<double>("NEGINTEGER", -123.0));
    ASSERT_TRUE(Parsed<double>("POSLONGINTEGER", 123456789000.0));
    ASSERT_TRUE(Parsed<double>("NEGLONGINTEGER", -123456789000.0));
    ASSERT_TRUE(Parsed<double>("POSDOUBLE", 123456.789));
    ASSERT_TRUE(Parsed<double>("NEGDOUBLE", -123456.789));

    ASSERT_TRUE(NotParsed<double>("STRING"));
}

TEST(Env, ParseAsFloat)
{
    SetEnv();

    ASSERT_TRUE(Parsed<float>("POSINTEGER", 123.0));
    ASSERT_TRUE(Parsed<float>("NEGINTEGER", -123.0));

    ASSERT_TRUE(NotParsed<float>("STRING"));
    ASSERT_TRUE(NotParsed<float>("POSLONGINTEGER"));
    ASSERT_TRUE(NotParsed<float>("NEGLONGINTEGER"));
    ASSERT_TRUE(NotParsed<float>("POSDOUBLE"));
    ASSERT_TRUE(NotParsed<float>("NEGDOUBLE"));
}

TEST(Env, ParseEmitValue)
{
    SetEnv();

    ASSERT_TRUE(Parsed<std::string>("STRING", "value"));
    ASSERT_TRUE(Parsed<int>("POSINTEGER", 123));
    ASSERT_TRUE(Parsed<int>("NEGINTEGER", -123));
    ASSERT_TRUE(Parsed<long int>("POSLONGINTEGER", 123456789000));
    ASSERT_TRUE(Parsed<long int>("NEGLONGINTEGER", -123456789000));
    ASSERT_TRUE(Parsed<double>("POSDOUBLE", 123456.789));
    ASSERT_TRUE(Parsed<double>("NEGDOUBLE", -123456.789));

    uconfig::EnvFormat format;
    std::map<std::string, std::string> env_dest;

    format.Emit<std::string>(&env_dest, "STRING", "value");
    format.Emit<int>(&env_dest, "POSINTEGER", 123);
    format.Emit<int>(&env_dest, "NEGINTEGER", -123);
    format.Emit<long int>(&env_dest, "POSLONGINTEGER", 123456789000);
    format.Emit<long int>(&env_dest, "NEGLONGINTEGER", -123456789000);
    format.Emit<double>(&env_dest, "POSDOUBLE", 123456.789);
    format.Emit<double>(&env_dest, "NEGDOUBLE", -123456.789);

    ASSERT_EQ(env_dest, env_source);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    ClearEnv();

    return result;
}
