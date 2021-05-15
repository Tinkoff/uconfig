#include "uconfig/Interface.h"
#include "gtest/gtest.h"

template <typename ConfigType, typename FormatType>
struct TypeParam
{
    using config_type = ConfigType;
    using format_type = FormatType;
};

template <typename TypeParam>
struct FormatParam
{
    using config_type = typename TypeParam::config_type;
    using format_type = typename TypeParam::format_type;
    using source_type = typename format_type::source_type;
    using dest_type = typename format_type::dest_type;

    virtual ~FormatParam() = default;

    virtual config_type Config() = 0;
    virtual source_type* Source() = 0;
    virtual void SetOptional(source_type*) = 0;
    virtual void SetMandatory(source_type*) = 0;
    virtual void SetAll(source_type*) = 0;
    virtual void Clear() = 0;
    virtual void EmitDefault(dest_type*) = 0;
    virtual void EmitOptional(dest_type*) = 0;
    virtual void EmitMandatory(dest_type*) = 0;
    virtual void EmitAll(dest_type*) = 0;
};

template <typename TypeParam>
struct Format: public ::testing::Test
{
    void TearDown() override
    {
        context->Clear();
    }

    static std::unique_ptr<FormatParam<TypeParam>> context;
};

TYPED_TEST_SUITE_P(Format);

#include "env.h"
#include "rapidjson.h"
