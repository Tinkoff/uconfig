#include "fixtures/format.h"

/* Config with another config in it */

static const int default_int = 300;

struct VarConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<int> int_var;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_INT", &int_var);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/int", &int_var);
    }
};

struct OptVarConfig: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    uconfig::Variable<int> int_var{default_int};

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_INT", &int_var);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/int", &int_var);
    }
};

struct Config: public uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>
{
    VarConfig var_config;
    VarConfig var_config_opt{true};
    OptVarConfig opt_var_config;

    using uconfig::Config<uconfig::EnvFormat, uconfig::RapidjsonFormat<>>::Config;

    virtual void Init(const std::string& config_path) override
    {
        Register<uconfig::EnvFormat>(config_path + "_NESTED", &var_config);
        Register<uconfig::EnvFormat>(config_path + "_OPT_NESTED", &var_config_opt);
        Register<uconfig::EnvFormat>(config_path + "_NESTED_OPT", &opt_var_config);

        Register<uconfig::RapidjsonFormat<>>(config_path + "/nested", &var_config);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/opt_nested", &var_config_opt);
        Register<uconfig::RapidjsonFormat<>>(config_path + "/nested_opt", &opt_var_config);
    }
};

struct EnvConfigParam: public EnvFormatParam<Config>
{
    virtual void EmitDefault(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("_NESTED_OPT_INT", "300");
    }

    virtual void EmitOptional(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("_OPT_NESTED_INT", "100");
        dst->emplace("_NESTED_OPT_INT", "200");
    }

    virtual void EmitMandatory(std::map<std::string, std::string>* dst) override
    {
        dst->emplace("_NESTED_INT", "123");
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

        rapidjson::Value nested_opt{rapidjson::kObjectType};
        nested_opt.AddMember("int", default_int, allocator);
        dst->AddMember("nested_opt", std::move(nested_opt), allocator);
    }

    virtual void EmitOptional(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();

        rapidjson::Value opt_nested{rapidjson::kObjectType};
        opt_nested.AddMember("int", (int)100, allocator);
        dst->AddMember("opt_nested", std::move(opt_nested), allocator);

        rapidjson::Value nested_opt{rapidjson::kObjectType};
        nested_opt.AddMember("int", (int)200, allocator);
        dst->AddMember("nested_opt", std::move(nested_opt), allocator);
    }

    virtual void EmitMandatory(rapidjson::Document* dst) override
    {
        if (!dst->IsObject()) {
            dst->SetObject();
        }
        auto& allocator = dst->GetAllocator();

        rapidjson::Value nested{rapidjson::kObjectType};
        nested.AddMember("int", (int)123, allocator);
        dst->AddMember("nested", std::move(nested), allocator);
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
    ASSERT_FALSE(config.var_config.Initialized());
    ASSERT_FALSE(config.var_config_opt.Initialized());
    ASSERT_TRUE(config.opt_var_config.Initialized());

    ASSERT_EQ(config.opt_var_config.int_var, default_int);

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
    ASSERT_FALSE(config.var_config.Initialized());
    ASSERT_TRUE(config.var_config_opt.Initialized());
    ASSERT_TRUE(config.opt_var_config.Initialized());

    ASSERT_EQ(config.var_config_opt.int_var, 100);
    ASSERT_EQ(config.opt_var_config.int_var, 200);

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
    ASSERT_TRUE(config.var_config.Initialized());
    ASSERT_FALSE(config.var_config_opt.Initialized());
    ASSERT_TRUE(config.opt_var_config.Initialized());

    ASSERT_EQ(config.var_config.int_var, 123);
    ASSERT_EQ(config.opt_var_config.int_var, default_int);

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
    ASSERT_TRUE(config.var_config.Initialized());
    ASSERT_TRUE(config.var_config_opt.Initialized());
    ASSERT_TRUE(config.opt_var_config.Initialized());

    ASSERT_EQ(config.var_config.int_var, 123);
    ASSERT_EQ(config.var_config_opt.int_var, 100);
    ASSERT_EQ(config.opt_var_config.int_var, 200);

    typename TypeParam::format_type::dest_type emit_dst;
    typename TypeParam::format_type::dest_type test_dst;
    ASSERT_NO_THROW(config.Emit(formatter, "", &emit_dst));
    Format<TypeParam>::context->EmitAll(&test_dst);

    ASSERT_EQ(emit_dst, test_dst);
}

REGISTER_TYPED_TEST_SUITE_P(Format, ParseNoValuesEmit, ParseNoMandatoryEmit, ParseOnlyMandatoryEmit, ParseAllEmit);

typedef ::testing::Types<EnvTypeParam<Config>, RapidjsonTypeParam<Config>> ConfigTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(NestedConfig, Format, ConfigTypes);

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
