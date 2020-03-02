#pragma once

#include "Interface.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

namespace uconfig {

template <typename AllocatorT>
class RapidjsonFormat: public BaseFormat
{
public:
    static inline const std::string name = "[JSON]";

    using allocator_type = AllocatorT;
    using json_doc_type = rapidjson::GenericDocument<rapidjson::UTF8<>, allocator_type>;
    using json_value_type = rapidjson::GenericValue<rapidjson::UTF8<>, allocator_type>;
    using json_pointer_type = rapidjson::GenericPointer<json_value_type, allocator_type>;
    using source_type = json_value_type;
    using dest_type = json_doc_type;

    template <typename DestT>
    boost::optional<DestT> Parse(const json_value_type* source, const std::string& path)
    {
        const auto* target = Get(source, path);
        if (!target) {
            return boost::none;
        }

        DestT result;
        if (!Convert<DestT>(*target, result)) {
            return boost::none;
        }
        return result;
    }

    template <typename ValueT>
    void Emit(const ValueT& value, const std::string& path, dest_type* dest)
    {
        Set(MakeJson(value, dest->GetAllocator()), path, dest);
    }

    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) override
    {
        return vector_path + "/" + std::to_string(index);
    }

private:
    const json_value_type* Get(const json_value_type* source, const std::string& path) const
    {
        return json_pointer_type(path).Get(*source);
    }

    void Set(json_value_type&& value, const std::string& path, dest_type* dest) const
    {
        json_pointer_type(path).Set(*dest, std::move(value), dest->GetAllocator());
    }

    template <typename DestT, typename std::enable_if<std::is_arithmetic<DestT>::value &&
                                                      !std::is_same<DestT, bool>::value>::type* = nullptr>
    bool Convert(const json_value_type& source, DestT& result)
    {
        switch (source.GetType()) {
        case rapidjson::kNumberType:
            if (source.IsDouble()) {
                result = (DestT)source.GetDouble();
            } else if (source.IsInt()) {
                result = (DestT)source.GetInt();
            } else if (source.IsUint()) {
                result = (DestT)source.GetUint();
            } else if (source.IsInt64()) {
                result = (DestT)source.GetInt64();
            } else {
                result = (DestT)source.GetUint64();
            }
            break;

        default:
            return false;
        }
        return true;
    }

    template <typename DestT, typename std::enable_if<std::is_same<DestT, std::string>::value>::type* = nullptr>
    bool Convert(const json_value_type& source, DestT& result)
    {
        switch (source.GetType()) {
        case rapidjson::kStringType:
            result = source.GetString();
            break;
        default:
            return false;
        }
        return true;
    }

    template <typename DestT, typename std::enable_if<std::is_same<DestT, bool>::value>::type* = nullptr>
    bool Convert(const json_value_type& source, DestT& result)
    {
        switch (source.GetType()) {
        case rapidjson::kTrueType:
        case rapidjson::kFalseType:
            result = source.GetBool();
            break;
        default:
            return false;
        }
        return true;
    }

    template <typename SrcT, typename std::enable_if<std::is_arithmetic<SrcT>::value>::type* = nullptr>
    json_value_type MakeJson(const SrcT& source, allocator_type&)
    {
        return json_value_type(source);
    }

    template <typename SrcT, typename std::enable_if<std::is_same<SrcT, std::string>::value>::type* = nullptr>
    json_value_type MakeJson(const SrcT& source, allocator_type& allocator)
    {
        return json_value_type(source, allocator);
    }
};

} // namespace uconfig
