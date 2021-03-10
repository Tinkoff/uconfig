#include "uconfig/format/Env.h"

#include <unordered_set>

template <typename ConfigType>
struct EnvTypeParam: public TypeParam<ConfigType, uconfig::EnvFormat>
{};

template <class ConfigType>
struct EnvFormatParam: public FormatParam<EnvTypeParam<ConfigType>>
{
    virtual ConfigType Config() override
    {
        return ConfigType{};
    }

    virtual void* Source() override
    {
        return nullptr;
    }

    virtual void SetOptional(void*) override
    {
        std::map<std::string, std::string> env_source;

        EmitOptional(&env_source);
        SetEnv(env_source);
    }

    virtual void SetMandatory(void*) override
    {
        std::map<std::string, std::string> env_source;

        EmitMandatory(&env_source);
        SetEnv(env_source);
    }

    virtual void SetAll(void* source) override
    {
        SetOptional(source);
        SetMandatory(source);
    }

    virtual void Clear() override
    {
        for (const auto& name : kEnvSetted) {
            unsetenv(name.c_str());
        }
    }

    virtual void EmitDefault(std::map<std::string, std::string>*) = 0;
    virtual void EmitOptional(std::map<std::string, std::string>*) = 0;
    virtual void EmitMandatory(std::map<std::string, std::string>*) = 0;

    virtual void EmitAll(std::map<std::string, std::string>* dst) override
    {
        EmitOptional(dst);
        EmitMandatory(dst);
    }

protected:
    static std::unordered_set<std::string> kEnvSetted;

    void SetEnv(const std::map<std::string, std::string>& env_source)
    {
        for (const auto& [name, value] : env_source) {
            setenv(name.c_str(), value.c_str(), 1);
            kEnvSetted.insert(name);
        }
    }
};

template <class ConfigType>
std::unordered_set<std::string> EnvFormatParam<ConfigType>::kEnvSetted{};

namespace std {

ostream& operator<<(ostream& out, const map<string, string>& env)
{
    for (const auto& [name, value] : env) {
        out << name << "=" << value << endl;
    }
    return out;
}

} // namespace std
