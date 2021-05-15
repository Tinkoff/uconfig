#pragma once

#include "Format.h"

// Enable std:string for rapidjson
#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif // RAPIDJSON_HAS_STDSTRING

// Disable RAPIDJSON_ASSERT for rapidjson
#ifndef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) \
    do {                    \
    } while (false)
#endif // RAPIDJSON_ASSERT

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace uconfig {

/**
 * Rapidjson format.
 *
 * @tparam AllocatorT rapidjson allocator to use. Default rapidjson::MemoryPoolAllocator<>.
 */
template <typename AllocatorT = rapidjson::MemoryPoolAllocator<>>
class RapidjsonFormat: public Format
{
public:
    /// Name of the format. Used to form nice error-strings.
    static inline const std::string name = "[JSON]";

    /// rapidjson allocator used.
    using allocator_type = AllocatorT;
    /// rapidjson::Document with @p allocator_type.
    using json_doc_type = rapidjson::GenericDocument<rapidjson::UTF8<>, allocator_type>;
    /// rapidjson::Value with @p allocator_type.
    using json_value_type = rapidjson::GenericValue<rapidjson::UTF8<>, allocator_type>;
    /// rapidjson::Pointer with @p allocator_type.
    using json_pointer_type = rapidjson::GenericPointer<json_value_type, allocator_type>;
    /// rapidjson::Value to parse from.
    using source_type = json_value_type;
    /// rapidjson::Document to emit to.
    using dest_type = json_doc_type;

    /**
     * Parse the value at @p path from @p source JSON.
     *
     * @tparam T Type to parse.
     *
     * @param[in] source JSON object to parse from.
     * @param[in] path JSON-path to the value.
     *
     * @returns Value wrapped in std::optional or std::nullopt.
     */
    template <typename T>
    std::optional<T> Parse(const json_value_type* source, const std::string& path) const;

    /**
     * Emit the value at @p path to JSON @p dest.
     *
     * @tparam T Type to emit.
     *
     * @param[in] dest JSON object to emit to.
     * @param[in] path JSON-path to the value.
     * @param[in] value Value to emit.
     */
    template <typename T>
    void Emit(dest_type* dest, const std::string& path, const T& value) const;

    /**
     * Construct JSON-path to a array element at @p index.
     *
     * @param[in] vector_path Path to the JSON-array itself.
     * @param[in] index Position in the array to make path to.
     *
     * @returns JSON-path to the array element at @p index.
     */
    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) const noexcept override;

private:
    /// Get the value from @p source at @p path.
    static const json_value_type* Get(const json_value_type* source, const std::string& path);
    /// Set the value int @p dest at @p path.
    static void Set(json_value_type&& value, const std::string& path, dest_type* dest);

    /// Convert JSON-value @p source into a std::string.
    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a bool.
    template <typename T, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a int.
    template <typename T, typename std::enable_if<std::is_same<T, int>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a long.
    template <typename T, typename std::enable_if<std::is_same<T, long>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a long long.
    template <typename T, typename std::enable_if<std::is_same<T, long long>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a unsigned.
    template <typename T, typename std::enable_if<std::is_same<T, unsigned>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a unsigned long.
    template <typename T, typename std::enable_if<std::is_same<T, unsigned long>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a unsigned long long.
    template <typename T, typename std::enable_if<std::is_same<T, unsigned long long>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a double.
    template <typename T, typename std::enable_if<std::is_same<T, double>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);
    /// Convert JSON-value @p source into a float.
    template <typename T, typename std::enable_if<std::is_same<T, float>::value>::type* = nullptr>
    static bool Convert(const json_value_type& source, T& result);

    // Helper to make a JSON-value from SrcT.
    template <typename SrcT, typename std::enable_if<
                                 std::is_same<SrcT, bool>::value || std::is_same<SrcT, int>::value ||
                                 std::is_same<SrcT, long long>::value || std::is_same<SrcT, unsigned>::value ||
                                 std::is_same<SrcT, unsigned long long>::value || std::is_same<SrcT, double>::value ||
                                 std::is_same<SrcT, float>::value>::type* = nullptr>
    static json_value_type MakeJson(const SrcT& source, allocator_type& alloc);
    // Helper to make a JSON-value from SrcT.
    template <typename SrcT, typename std::enable_if<std::is_same<SrcT, long>::value>::type* = nullptr>
    static json_value_type MakeJson(const SrcT& source, allocator_type& alloc);
    // Helper to make a JSON-value from SrcT.
    template <typename SrcT, typename std::enable_if<std::is_same<SrcT, unsigned long>::value>::type* = nullptr>
    static json_value_type MakeJson(const SrcT& source, allocator_type& alloc);
    // Helper to make a JSON-value from SrcT.
    template <typename SrcT, typename std::enable_if<std::is_same<SrcT, std::string>::value>::type* = nullptr>
    static json_value_type MakeJson(const SrcT& source, allocator_type& alloc);
};

} // namespace uconfig

#include "impl/Rapidjson.ipp"
