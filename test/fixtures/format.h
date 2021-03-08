#include "uconfig/Interface.h"
#include "gtest/gtest.h"

template <class ConfigType>
struct FormatParam
{
    using source_type = typename ConfigType::format_type::source_type;
    using dest_type = typename ConfigType::format_type::dest_type;

    virtual ConfigType Config() = 0;
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

template <class ConfigType>
struct Format: public ::testing::Test
{
    void TearDown() override
    {
        context->Clear();
    }

    static std::unique_ptr<FormatParam<ConfigType>> context;
};

TYPED_TEST_CASE_P(Format);

#include "env.h"
#include "rapidjson.h"
