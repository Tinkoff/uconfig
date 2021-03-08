#pragma once

#include <optional>
#include <string>

namespace uconfig {

/// Abstract format interface.
class Format
{
public:
    /// Name of the format. Used to form nice error-strings.
    static inline const std::string name = "[NO FORMAT]";
    /// Source of the format to parse from.
    using source_type = void;
    /// Destination of the format to emit to.
    using dest_type = void;

    /**
     * Parse the value at @p path from @p source.
     *
     * @tparam T Type to parse.
     *
     * @param[in] source Source to parse value from.
     * @param[in] path Path where the value resides in @p source.
     *
     * @returns Value wrapped in std::optional or std::nullopt.
     */
    template <typename T>
    std::optional<T> Parse(const source_type* source, const std::string& path) const;

    /**
     * Emit the value at @p path to @p dest.
     *
     * @tparam T Type to emit.
     *
     * @param[in] dest Destination to emit to.
     * @param[in] path Path where to emit.
     * @param[in] value Value to emit.
     */
    template <typename ValueT>
    void Emit(dest_type* dest, const std::string& path, const ValueT& value) const;

    /**
     * Construct path to a vector element at @p index accroding to the format.
     *
     * @param[in] vector_path Path to the vector itself.
     * @param[in] index Position in the vector to make path to.
     *
     * @returns Path to the vector element at @p index.
     */
    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) const noexcept = 0;
};

} // namespace uconfig
