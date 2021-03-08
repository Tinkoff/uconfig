#include "fixtures/format.h"

/* Config with vectors in it */

static const std::vector<int> default_empty_vector{};
static const std::vector<int> default_vector{1, 2, 3};

struct EnvConfig: public uconfig::Config<uconfig::EnvFormat>
{
    uconfig::Vector<int> vector;
    uconfig::Vector<int> optional_empty_vector{true};
    uconfig::Vector<int> optional_default_vector{default_vector};

    using format_type = uconfig::EnvFormat;

    using uconfig::Config<uconfig::EnvFormat>::Config;

    virtual void Init(const std::string&) override
    {
        Register("VECTOR", &vector);
        Register("OPT_VECTOR", &optional_empty_vector);
        Register("OPT_DEF_VECTOR", &optional_default_vector);
    }
};

struct EnvConfigParam: public EnvParam<EnvConfig>
{
    virtual void EmitDefault(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("OPT_DEF_VECTOR_0", "1");
        dst->emplace("OPT_DEF_VECTOR_1", "2");
        dst->emplace("OPT_DEF_VECTOR_2", "3");
    }

    virtual void EmitOptional(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("OPT_VECTOR_0", "11");
        dst->emplace("OPT_VECTOR_1", "22");
        dst->emplace("OPT_VECTOR_2", "33");
        dst->emplace("OPT_DEF_VECTOR_0", "44");
        dst->emplace("OPT_DEF_VECTOR_1", "55");
        dst->emplace("OPT_DEF_VECTOR_2", "66");
    }

    virtual void EmitMandatory(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("VECTOR_0", "123");
        dst->emplace("VECTOR_1", "456");
        dst->emplace("VECTOR_2", "789");
    }
};

struct RapidjsonConfig: public uconfig::Config<uconfig::RapidjsonFormat<>>
{
    uconfig::Vector<int> vector;
    uconfig::Vector<int> optional_empty_vector{true};
    uconfig::Vector<int> optional_default_vector{default_vector};

    using format_type = uconfig::RapidjsonFormat<>;

    using uconfig::Config<uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
        Register("/vector", &vector);
        Register("/opt_vector", &optional_empty_vector);
        Register("/opt_def_vector", &optional_default_vector);
    }
};

struct RapidjsonConfigParam: public RapidjsonParam<RapidjsonConfig>
{
    virtual void EmitDefault(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();

        rapidjson::Value opt_def_vector{rapidjson::kArrayType};
        for (const int i : default_vector) {
            opt_def_vector.PushBack(i, allocator);
        }
        dst->AddMember("opt_def_vector", std::move(opt_def_vector), allocator);
    }

    virtual void EmitOptional(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();

        rapidjson::Value opt_vector{rapidjson::kArrayType};
        opt_vector.PushBack(int(11), allocator);
        opt_vector.PushBack(int(22), allocator);
        opt_vector.PushBack(int(33), allocator);
        dst->AddMember("opt_vector", std::move(opt_vector), allocator);

        rapidjson::Value opt_def_vector{rapidjson::kArrayType};
        opt_def_vector.PushBack(int(44), allocator);
        opt_def_vector.PushBack(int(55), allocator);
        opt_def_vector.PushBack(int(66), allocator);
        dst->AddMember("opt_def_vector", std::move(opt_def_vector), allocator);
    }

    virtual void EmitMandatory(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();

        rapidjson::Value vector{rapidjson::kArrayType};
        vector.PushBack(int(123), allocator);
        vector.PushBack(int(456), allocator);
        vector.PushBack(int(789), allocator);
        dst->AddMember("vector", std::move(vector), allocator);
    }
};

TYPED_TEST_P(Format, ParseNoValuesEmit)
{
    typename TypeParam::format_type formatter;
    auto config = Format<TypeParam>::context->Config();
    auto* source = Format<TypeParam>::context->Source();

    ASSERT_THROW(config.Parse(formatter, "", source), uconfig::ParseError);
    ASSERT_FALSE(config.Parse(formatter, "", source, false));

    ASSERT_FALSE(config.Initialized());
    ASSERT_FALSE(config.vector.Initialized());
    ASSERT_TRUE(config.optional_empty_vector.Initialized());
    ASSERT_TRUE(config.optional_default_vector.Initialized());

    ASSERT_EQ(config.optional_empty_vector, default_empty_vector);
    ASSERT_EQ(config.optional_default_vector, default_vector);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_THROW(config.Emit(formatter, "", &emit_dst), uconfig::EmitError);
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst, false));

    Format<TypeParam>::context->EmitDefault(&test_dst);
    ASSERT_EQ(emit_dst, test_dst);
}

TYPED_TEST_P(Format, ParseNoMandatoryEmit)
{
    typename TypeParam::format_type formatter;
    auto config = Format<TypeParam>::context->Config();
    auto* source = Format<TypeParam>::context->Source();
    Format<TypeParam>::context->SetOptional(source);

    ASSERT_TRUE(config.Parse(formatter, "", source, false));

    ASSERT_FALSE(config.Initialized());
    ASSERT_FALSE(config.vector.Initialized());
    ASSERT_TRUE(config.optional_empty_vector.Initialized());
    ASSERT_TRUE(config.optional_default_vector.Initialized());

    const auto test_opt_vector = std::vector<int>{11, 22, 33};
    const auto test_opt_def_vector = std::vector<int>{44, 55, 66};
    ASSERT_EQ(config.optional_empty_vector, test_opt_vector);
    ASSERT_EQ(config.optional_default_vector, test_opt_def_vector);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst, false));
    Format<TypeParam>::context->EmitOptional(&test_dst);

    ASSERT_EQ(emit_dst, test_dst);
}

TYPED_TEST_P(Format, ParseOnlyMandatoryEmit)
{
    typename TypeParam::format_type formatter;
    auto config = Format<TypeParam>::context->Config();
    auto* source = Format<TypeParam>::context->Source();
    Format<TypeParam>::context->SetMandatory(source);

    ASSERT_TRUE(config.Parse(formatter, "", source));
    ASSERT_TRUE(config.Initialized());
    ASSERT_TRUE(config.vector.Initialized());
    ASSERT_TRUE(config.optional_empty_vector.Initialized());
    ASSERT_TRUE(config.optional_default_vector.Initialized());

    const auto test_vector = std::vector<int>{123, 456, 789};
    ASSERT_EQ(config.vector, test_vector);
    ASSERT_EQ(config.optional_empty_vector, default_empty_vector);
    ASSERT_EQ(config.optional_default_vector, default_vector);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst));
    Format<TypeParam>::context->EmitDefault(&test_dst);
    Format<TypeParam>::context->EmitMandatory(&test_dst);

    ASSERT_EQ(emit_dst, test_dst);
}

TYPED_TEST_P(Format, ParseAllEmit)
{
    typename TypeParam::format_type formatter;
    auto config = Format<TypeParam>::context->Config();
    auto* source = Format<TypeParam>::context->Source();
    Format<TypeParam>::context->SetAll(source);

    ASSERT_TRUE(config.Parse(formatter, "", source));
    ASSERT_TRUE(config.Initialized());

    const auto test_vector = std::vector<int>{123, 456, 789};
    const auto test_opt_vector = std::vector<int>{11, 22, 33};
    const auto test_opt_def_vector = std::vector<int>{44, 55, 66};
    ASSERT_EQ(config.vector, test_vector);
    ASSERT_EQ(config.optional_empty_vector, test_opt_vector);
    ASSERT_EQ(config.optional_default_vector, test_opt_def_vector);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst));
    Format<TypeParam>::context->EmitAll(&test_dst);

    ASSERT_EQ(emit_dst, test_dst);
}

REGISTER_TYPED_TEST_CASE_P(Format, ParseNoValuesEmit, ParseNoMandatoryEmit, ParseOnlyMandatoryEmit, ParseAllEmit);

typedef ::testing::Types<EnvConfig, RapidjsonConfig> ConfigTypes;
INSTANTIATE_TYPED_TEST_CASE_P(VectorConfig, Format, ConfigTypes);

// clang-format off
template <>
std::unique_ptr<FormatParam<EnvConfig>> Format<EnvConfig>::context = std::make_unique<EnvConfigParam>();
template <>
std::unique_ptr<FormatParam<RapidjsonConfig>> Format<RapidjsonConfig>::context = std::make_unique<RapidjsonConfigParam>();
// clang-format on

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
