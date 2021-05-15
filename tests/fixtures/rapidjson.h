#include "uconfig/format/Rapidjson.h"

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

template <typename ConfigType>
struct RapidjsonTypeParam: public TypeParam<ConfigType, uconfig::RapidjsonFormat<>>
{};

template <class ConfigType>
struct RapidjsonFormatParam: public FormatParam<RapidjsonTypeParam<ConfigType>>
{
    virtual ConfigType Config() override
    {
        return ConfigType{};
    }

    virtual rapidjson::Value* Source() override
    {
        return &kJsonSource;
    }

    virtual void SetOptional(rapidjson::Value* source) override
    {
        ASSERT_EQ(source, &kJsonSource);
        EmitOptional(&kJsonSource);
    }

    virtual void SetMandatory(rapidjson::Value* source) override
    {
        ASSERT_EQ(source, &kJsonSource);
        EmitMandatory(&kJsonSource);
    }

    virtual void SetAll(rapidjson::Value* source) override
    {
        SetOptional(source);
        SetMandatory(source);
    }

    virtual void Clear() override
    {
        kJsonSource.RemoveAllMembers();
    }

    virtual void EmitDefault(rapidjson::Document*) override = 0;
    virtual void EmitOptional(rapidjson::Document*) override = 0;
    virtual void EmitMandatory(rapidjson::Document*) override = 0;

    virtual void EmitAll(rapidjson::Document* dst) override
    {
        EmitOptional(dst);
        EmitMandatory(dst);
    }

protected:
    static rapidjson::Document kJsonSource;
};

template <class ConfigType>
rapidjson::Document RapidjsonFormatParam<ConfigType>::kJsonSource{rapidjson::kObjectType};

namespace rapidjson {

std::ostream& operator<<(std::ostream& out, const Value& json)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    json.Accept(writer);
    return out << buffer.GetString();
}

} // namespace rapidjson
