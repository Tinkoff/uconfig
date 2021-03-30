#pragma once

namespace uconfig {

template <typename AllocatorT>
template <typename T>
std::optional<T> RapidjsonFormat<AllocatorT>::Parse(const json_value_type* source, const std::string& path) const
{
    const auto* target = Get(source, path);
    if (!target) {
        return std::nullopt;
    }

    T result;
    if (!Convert<T>(*target, result)) {
        return std::nullopt;
    }
    return result;
}

template <typename AllocatorT>
template <typename T>
void RapidjsonFormat<AllocatorT>::Emit(dest_type* dest, const std::string& path, const T& value) const
{
    Set(MakeJson(value, dest->GetAllocator()), path, dest);
}

template <typename AllocatorT>
std::string RapidjsonFormat<AllocatorT>::VectorElementPath(const std::string& vector_path,
                                                           std::size_t index) const noexcept
{
    return vector_path + "/" + std::to_string(index);
}

template <typename AllocatorT>
const typename RapidjsonFormat<AllocatorT>::json_value_type*
RapidjsonFormat<AllocatorT>::Get(const json_value_type* source, const std::string& path)
{
    return json_pointer_type(path).Get(*source);
}

template <typename AllocatorT>
void RapidjsonFormat<AllocatorT>::Set(json_value_type&& value, const std::string& path, dest_type* dest)
{
    json_pointer_type(path).Set(*dest, std::move(value), dest->GetAllocator());
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
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

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, bool>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
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

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, int>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsInt()) {
        result = source.GetInt();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, int64_t>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsInt64()) {
        result = source.GetInt64();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, unsigned>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsUint()) {
        result = source.GetUint();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, uint64_t>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsUint64()) {
        result = source.GetUint64();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, double>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsLosslessDouble()) {
        result = source.GetDouble();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename T, typename std::enable_if<std::is_same<T, float>::value>::type*>
bool RapidjsonFormat<AllocatorT>::Convert(const json_value_type& source, T& result)
{
    if (source.IsLosslessFloat()) {
        result = source.GetFloat();
        return true;
    }

    return false;
}

template <typename AllocatorT>
template <typename SrcT, typename std::enable_if<!std::is_same<SrcT, std::string>::value>::type*>
typename RapidjsonFormat<AllocatorT>::json_value_type RapidjsonFormat<AllocatorT>::MakeJson(const SrcT& source,
                                                                                            allocator_type& /*alloc*/)
{
    return json_value_type(source);
}

template <typename AllocatorT>
template <typename SrcT, typename std::enable_if<std::is_same<SrcT, std::string>::value>::type*>
typename RapidjsonFormat<AllocatorT>::json_value_type RapidjsonFormat<AllocatorT>::MakeJson(const SrcT& source,
                                                                                            allocator_type& alloc)
{
    return json_value_type(source, alloc);
}

} // namespace uconfig
