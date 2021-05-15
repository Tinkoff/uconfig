#pragma once

#include "Format.h"

#include <map>

namespace uconfig {

/**
 * Environment format.
 * Parsed values from env, emit into `std::map`.
 */
class EnvFormat: public Format
{
public:
    /// Name of the format. Used to form nice error-strings.
    static inline const std::string name = "[ENV]";
    /// Env does't have a source type.
    using source_type = void;
    /// `std::map<std::string, std::string>` to emit to.
    using dest_type = std::map<std::string, std::string>;

    /**
     * Parse the value with name @p path from environment.
     *
     * @tparam T Type to parse.
     *
     * @param[in] path Name of the value.
     *
     * @returns Value wrapped in std::optional or std::nullopt.
     */
    template <typename T>
    std::optional<T> Parse(const source_type* /*unused*/, const std::string& path) const;

    /**
     * Emit the (@p path, @p value) pair into @p dest.
     *
     * @tparam T Type to emit.
     *
     * @param[in] dest Map to emit to.
     * @param[in] path Key to emplace in the map.
     * @param[in] value Value to emit.
     */
    template <typename T>
    void Emit(dest_type* dest, const std::string& path, const T& value) const;

    /**
     * Construct array element name using '_' as delimiter.
     *
     * @param[in] vector_path Name array itself.
     * @param[in] index Position in the array to make path to.
     *
     * @returns '_' delimited name to the element at @p index, e.g. "ARRAY_0"
     *  for @p vector_path = "ARRAY" and @p index = 0.
     */
    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) const noexcept override;

private:
    /// Convert std::string to `T`.
    template <typename T>
    static std::optional<T> FromString(const std::string& str);

    /// Convert `T` to std::string.
    template <typename T>
    static std::string ToString(const T& value);
};

} // namespace uconfig

#include "impl/Env.ipp"
