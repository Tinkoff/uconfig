#pragma once

#include <cstdlib>
#include <limits>
#include <sstream>

namespace uconfig {

template <typename T>
std::optional<T> EnvFormat::Parse(const source_type*, const std::string& path) const
{
    const char* env_var = std::getenv(path.c_str());
    if (!env_var) {
        return std::nullopt;
    }
    return FromString<T>(env_var);
}

template <typename T>
void EnvFormat::Emit(dest_type* dest, const std::string& path, const T& value) const
{
    dest->emplace(std::make_pair(path, ToString<T>(value)));
}

std::string EnvFormat::VectorElementPath(const std::string& vector_path, std::size_t index) const noexcept
{
    return vector_path + "_" + std::to_string(index);
}

template <typename T>
std::optional<T> EnvFormat::FromString(const std::string& str)
{
    T result;

    std::istringstream ss(str);
    ss >> result;
    if (ss.fail() || ToString(result) != str) {
        return std::nullopt;
    }

    return result;
}

template <typename T>
std::string EnvFormat::ToString(const T& value)
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

template <>
inline std::optional<std::string> EnvFormat::FromString<std::string>(const std::string& str)
{
    return str;
}

template <>
inline std::string EnvFormat::ToString<std::string>(const std::string& value)
{
    return value;
}

} // namespace uconfig
