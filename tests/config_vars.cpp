#include "fixtures/format.h"

/* Config with variables in it */

struct Config: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<int> int_var;
    uconfig::Variable<double> double_var;
    uconfig::Variable<std::string> str_var;
    uconfig::Variable<long int> longint_var;
    uconfig::Variable<int> optional_int_var{111};

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string&) override
    {
        Register<uconfig::EnvFormat>("INT", &int_var);
        Register<uconfig::EnvFormat>("DOUBLE", &double_var);
        Register<uconfig::EnvFormat>("STRING", &str_var);
        Register<uconfig::EnvFormat>("LONGINT", &longint_var);
        Register<uconfig::EnvFormat>("OPT_INT", &optional_int_var);

        Register<uconfig::RapidjsonFormat<>>("/int", &int_var);
        Register<uconfig::RapidjsonFormat<>>("/double", &double_var);
        Register<uconfig::RapidjsonFormat<>>("/string", &str_var);
        Register<uconfig::RapidjsonFormat<>>("/longint", &longint_var);
        Register<uconfig::RapidjsonFormat<>>("/opt_int", &optional_int_var);
    }
};

struct EnvConfigParam: public EnvFormatParam<Config>
{
    virtual void EmitDefault(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("OPT_INT", "111");
    }

    virtual void EmitOptional(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("OPT_INT", "6700");
    }

    virtual void EmitMandatory(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("STRING", "value");
        dst->emplace("INT", "123");
        dst->emplace("LONGINT", "123456789000");
        dst->emplace("DOUBLE", "123456.789");
    }
};

struct RapidjsonConfigParam: public RapidjsonFormatParam<Config>
{
    virtual void EmitDefault(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();
        dst->AddMember("opt_int", (int)111, allocator);
    }

    virtual void EmitOptional(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();
        dst->AddMember("opt_int", (int)6700, allocator);
    }

    virtual void EmitMandatory(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();
        dst->AddMember("string", rapidjson::Value("value", allocator), allocator);
        dst->AddMember("int", (int)123, allocator);
        dst->AddMember("longint", (int64_t)123456789000, allocator);
        dst->AddMember("double", (double)123456.789, allocator);
    }
};

TYPED_TEST_P(Format, ParseNoValuesEmit)
{
    typename TypeParam::format_type formatter;
    auto config = Format<TypeParam>::context->Config();
    auto* source = Format<TypeParam>::context->Source();

    ASSERT_FALSE(config.Initialized());
    ASSERT_THROW(config.Parse(formatter, "", source), uconfig::ParseError);
    ASSERT_FALSE(config.Parse(formatter, "", source, false));

    ASSERT_FALSE(config.Initialized());
    ASSERT_FALSE(config.int_var.Initialized());
    ASSERT_FALSE(config.double_var.Initialized());
    ASSERT_FALSE(config.str_var.Initialized());
    ASSERT_FALSE(config.longint_var.Initialized());
    ASSERT_TRUE(config.optional_int_var.Initialized());
    ASSERT_EQ(config.optional_int_var, 111);

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

    ASSERT_FALSE(config.Initialized());
    ASSERT_TRUE(config.Parse(formatter, "", source, false));

    ASSERT_FALSE(config.Initialized());
    ASSERT_FALSE(config.int_var.Initialized());
    ASSERT_FALSE(config.double_var.Initialized());
    ASSERT_FALSE(config.str_var.Initialized());
    ASSERT_FALSE(config.longint_var.Initialized());
    ASSERT_TRUE(config.optional_int_var.Initialized());
    ASSERT_EQ(config.optional_int_var, 6700);

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

    ASSERT_FALSE(config.Initialized());
    ASSERT_TRUE(config.Parse(formatter, "", source));
    ASSERT_TRUE(config.Initialized());

    ASSERT_EQ(config.int_var, 123);
    ASSERT_EQ(config.double_var, 123456.789);
    ASSERT_EQ(config.str_var, "value");
    ASSERT_EQ(config.longint_var, 123456789000);
    ASSERT_EQ(config.optional_int_var, 111);

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

    ASSERT_FALSE(config.Initialized());
    ASSERT_TRUE(config.Parse(formatter, "", source));
    ASSERT_TRUE(config.Initialized());

    ASSERT_EQ(config.int_var, 123);
    ASSERT_EQ(config.double_var, 123456.789);
    ASSERT_EQ(config.str_var, "value");
    ASSERT_EQ(config.longint_var, 123456789000);
    ASSERT_EQ(config.optional_int_var, 6700);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst));
    Format<TypeParam>::context->EmitAll(&test_dst);

    ASSERT_EQ(emit_dst, test_dst);
}

REGISTER_TYPED_TEST_SUITE_P(Format, ParseNoValuesEmit, ParseNoMandatoryEmit, ParseOnlyMandatoryEmit, ParseAllEmit);

typedef ::testing::Types<EnvTypeParam<Config>, RapidjsonTypeParam<Config>> ConfigTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(VarsConfig, Format, ConfigTypes);

// clang-format off
template <>
std::unique_ptr<FormatParam<EnvTypeParam<Config>>> Format<EnvTypeParam<Config>>::context = std::make_unique<EnvConfigParam>();
template <>
std::unique_ptr<FormatParam<RapidjsonTypeParam<Config>>> Format<RapidjsonTypeParam<Config>>::context = std::make_unique<RapidjsonConfigParam>();
// clang-format on

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
