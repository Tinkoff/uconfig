#include "uconfig/format/Rapidjson.h"
#include "gtest/gtest.h"

/* Since json values are typed we parse only correct ones */

const rapidjson::Document SetJson()
{
    rapidjson::Document json{rapidjson::kObjectType};

    auto& allocator = json.GetAllocator();
    json.RemoveAllMembers();
    json.AddMember("string", rapidjson::Value("value", allocator), allocator);
    json.AddMember("posinteger", (int)123, allocator);
    json.AddMember("neginteger", (int)-123, allocator);
    json.AddMember("poslonginteger", (int64_t)123456789000, allocator);
    json.AddMember("neglonginteger", (int64_t)-123456789000, allocator);
    json.AddMember("posdouble", (double)123456.789, allocator);
    json.AddMember("negdouble", (double)-123456.789, allocator);

    return json;
}

const rapidjson::Document ClearJson()
{
    return rapidjson::Document(rapidjson::kObjectType);
}

template <typename T>
testing::AssertionResult Parsed(const rapidjson::Value& json, const std::string& path, const T& expected_value)
{
    std::optional<T> value = uconfig::RapidjsonFormat<>{}.Parse<T>(&json, path);
    if (!value.has_value()) {
        return ::testing::AssertionFailure() << "'" << path << "' json variable was not parsed";
    }

    if (*value != expected_value) {
        return ::testing::AssertionFailure() << "'" << path << "' json variable value '" << *value
                                             << "' differs from expected '" << expected_value << "'";
    }

    return testing::AssertionSuccess() << "'" << path << "' json variable was parsed with value" << *value;
}

template <typename T>
testing::AssertionResult NotParsed(const rapidjson::Value& json, const std::string& path)
{
    std::optional<T> value = uconfig::RapidjsonFormat<>{}.Parse<T>(&json, path);
    if (value.has_value()) {
        return ::testing::AssertionFailure() << "'" << path << "' json variable was parsed";
    }

    return testing::AssertionSuccess();
}

TEST(Rapidjson, ParseNoValue)
{
    const auto json = ClearJson();

    ASSERT_TRUE(NotParsed<std::string>(json, "/string"));
    ASSERT_TRUE(NotParsed<int>(json, "/posinteger"));
    ASSERT_TRUE(NotParsed<int>(json, "/neginteger"));
    ASSERT_TRUE(NotParsed<long int>(json, "/poslonginteger"));
    ASSERT_TRUE(NotParsed<long int>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<double>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<double>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsString)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<std::string>(json, "/string", "value"));

    ASSERT_TRUE(NotParsed<std::string>(json, "/posinteger"));
    ASSERT_TRUE(NotParsed<std::string>(json, "/neginteger"));
    ASSERT_TRUE(NotParsed<std::string>(json, "/poslonginteger"));
    ASSERT_TRUE(NotParsed<std::string>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<std::string>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<std::string>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsInt)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<int>(json, "/posinteger", 123));
    ASSERT_TRUE(Parsed<int>(json, "/neginteger", -123));

    ASSERT_TRUE(NotParsed<int>(json, "/string"));
    ASSERT_TRUE(NotParsed<int>(json, "/poslonginteger"));
    ASSERT_TRUE(NotParsed<int>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<int>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<int>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsUnsignedInt)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<unsigned>(json, "/posinteger", 123));

    ASSERT_TRUE(NotParsed<unsigned>(json, "/neginteger"));
    ASSERT_TRUE(NotParsed<unsigned>(json, "/string"));
    ASSERT_TRUE(NotParsed<unsigned>(json, "/poslonginteger"));
    ASSERT_TRUE(NotParsed<unsigned>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<unsigned>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<unsigned>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsLongInt)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<long int>(json, "/posinteger", 123));
    ASSERT_TRUE(Parsed<long int>(json, "/neginteger", -123));
    ASSERT_TRUE(Parsed<long int>(json, "/poslonginteger", 123456789000));
    ASSERT_TRUE(Parsed<long int>(json, "/neglonginteger", -123456789000));

    ASSERT_TRUE(NotParsed<long int>(json, "/string"));
    ASSERT_TRUE(NotParsed<long int>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<long int>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsUnsignedLongInt)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<unsigned long int>(json, "/posinteger", 123));
    ASSERT_TRUE(Parsed<unsigned long int>(json, "/poslonginteger", 123456789000));

    ASSERT_TRUE(NotParsed<unsigned long int>(json, "/string"));
    ASSERT_TRUE(NotParsed<unsigned long int>(json, "/neginteger"));
    ASSERT_TRUE(NotParsed<unsigned long int>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<unsigned long int>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<unsigned long int>(json, "/negdouble"));
}

TEST(Rapidjson, ParseAsDouble)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<double>(json, "/posinteger", 123.0));
    ASSERT_TRUE(Parsed<double>(json, "/neginteger", -123.0));
    ASSERT_TRUE(Parsed<double>(json, "/poslonginteger", 123456789000.0));
    ASSERT_TRUE(Parsed<double>(json, "/neglonginteger", -123456789000.0));
    ASSERT_TRUE(Parsed<double>(json, "/posdouble", 123456.789));
    ASSERT_TRUE(Parsed<double>(json, "/negdouble", -123456.789));

    ASSERT_TRUE(NotParsed<double>(json, "/string"));
}

TEST(Rapidjson, ParseAsFloat)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<float>(json, "/posinteger", 123.0));
    ASSERT_TRUE(Parsed<float>(json, "/neginteger", -123.0));

    ASSERT_TRUE(NotParsed<float>(json, "/string"));
    ASSERT_TRUE(NotParsed<float>(json, "/poslonginteger"));
    ASSERT_TRUE(NotParsed<float>(json, "/neglonginteger"));
    ASSERT_TRUE(NotParsed<float>(json, "/posdouble"));
    ASSERT_TRUE(NotParsed<float>(json, "/negdouble"));
}

TEST(Rapidjson, ParseEmitValue)
{
    const auto json = SetJson();

    ASSERT_TRUE(Parsed<std::string>(json, "/string", "value"));
    ASSERT_TRUE(Parsed<int>(json, "/posinteger", 123));
    ASSERT_TRUE(Parsed<int>(json, "/neginteger", -123));
    ASSERT_TRUE(Parsed<long int>(json, "/poslonginteger", 123456789000));
    ASSERT_TRUE(Parsed<long int>(json, "/neglonginteger", -123456789000));
    ASSERT_TRUE(Parsed<double>(json, "/posdouble", 123456.789));
    ASSERT_TRUE(Parsed<double>(json, "/negdouble", -123456.789));

    uconfig::RapidjsonFormat<> format;
    rapidjson::Document json_dest;

    format.Emit<std::string>(&json_dest, "/string", "value");
    format.Emit<int>(&json_dest, "/posinteger", 123);
    format.Emit<int>(&json_dest, "/neginteger", -123);
    format.Emit<long int>(&json_dest, "/poslonginteger", 123456789000);
    format.Emit<long int>(&json_dest, "/neglonginteger", -123456789000);
    format.Emit<double>(&json_dest, "/posdouble", 123456.789);
    format.Emit<double>(&json_dest, "/negdouble", -123456.789);

    ASSERT_EQ(json_dest, json);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
