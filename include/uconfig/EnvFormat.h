#pragma once

#include "Interface.h"

#include <map>
#include <sstream>

namespace uconfig {

class EnvFormat: public BaseFormat
{
public:
    static inline const std::string name = "[ENV]";

    using source_type = void;
    using dest_type = std::map<std::string, std::string>;

    template <typename DestT>
    boost::optional<DestT> Parse(const source_type* source, const std::string& path)
    {
        const char* env_var = std::getenv(path.c_str());
        if (!env_var) {
            return boost::none;
        }
        return FromString<DestT>(env_var);
    }

    template <typename ValueT>
    void Emit(const ValueT& value, const std::string& path, dest_type* dest)
    {
        dest->emplace(std::make_pair(path, ToString<ValueT>(value)));
    }

    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) override
    {
        return vector_path + "_" + std::to_string(index);
    }

private:
    template <typename T>
    boost::optional<T> FromString(const std::string& str) const
    {
        T result;

        std::istringstream ss(str);
        ss >> result;
        if (ss.fail()) {
            return boost::none;
        }

        return result;
    }

    template <typename T>
    std::string ToString(const T& value) const
    {
        std::ostringstream ss;
        // by default stream print with .3 digits accuracy, so 123456.0 -> "1.234e+05" -> 123400;
        // but full precision (max_digits10) leads to: 11.0/10.0 -> 1.1000000000000001
        // [https://coliru.stacked-crooked.com/a/1681ce5326697585]
        if (std::numeric_limits<T>::max_digits10 > 0) {
            ss.precision(std::numeric_limits<T>::max_digits10 - 1);
        }

        ss << value;
        if (ss.fail()) {
            throw std::runtime_error("failed to print value");
        }
        return ss.str();
    }
};

} // namespace uconfig
