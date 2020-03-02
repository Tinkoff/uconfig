#pragma once

#include <boost/optional.hpp>

#include <list>
#include <memory>
#include <ostream>
#include <vector>

namespace uconfig {

template <typename Format>
class Interface;

template <typename T, typename Format>
class VariableIface;

template <typename Format>
class SectionIface;

template <typename T, typename Format>
class VectorIface;

struct ParseError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <typename T>
class Variable
{
public:
    template <typename F>
    using iface_type = VariableIface<T, F>;

    Variable()
        : value_(boost::none)
    {
    }

    Variable(T&& initialized_value)
        : value_(std::move(initialized_value))
    {
    }

    Variable(const Variable<T>&) = default;
    Variable<T>& operator=(const Variable<T>&) = default;

    Variable(Variable<T>&& other) noexcept
        : value_(std::move(other.value_))
    {
    }

    Variable<T>& operator=(Variable<T>&& other) noexcept
    {
        value_ = std::move(other.value_);
        return *this;
    }

    Variable<T>& operator=(T&& other) noexcept
    {
        value_ = std::move(other);
        return *this;
    }

    virtual ~Variable() = default;

    const T& Get() const
    {
        if (!Initialized()) {
            throw std::runtime_error("config variable is not set");
        }
        return *value_;
    }

    bool Initialized() const { return value_ != boost::none; }

    operator const T&() const { return Get(); }

    const T& operator*() const { return Get(); }

    const T* operator->() const { return &Get(); }

protected:
    boost::optional<T> value_;
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const Variable<T>& var)
{
    if (!var.Initialized()) {
        return out << "[not set]";
    }
    return out << var.Get();
}

template <typename T>
T operator+(const T& lhs, const Variable<T>& var)
{
    return lhs + var.Get();
}

inline std::string operator+(const char* lhs, const Variable<std::string>& var)
{
    return std::string(lhs) + var.Get();
}

template <typename T>
T operator-(const T& lhs, const Variable<T>& var)
{
    return lhs - var.Get();
}

template <typename T>
bool operator==(const T& lhs, const Variable<T>& var)
{
    return lhs == var.Get();
}

template <typename T>
bool operator!=(const T& lhs, const Variable<T>& var)
{
    return lhs != var.Get();
}

template <typename T>
bool operator>(const T& lhs, const Variable<T>& var)
{
    return lhs > var.Get();
}

template <typename T>
bool operator<(const T& lhs, const Variable<T>& var)
{
    return lhs < var.Get();
}

template <typename T>
bool operator>=(const T& lhs, const Variable<T>& var)
{
    return lhs >= var.Get();
}

template <typename T>
bool operator<=(const T& lhs, const Variable<T>& var)
{
    return lhs <= var.Get();
}

template <typename T>
T operator+(const Variable<T>& var, const T& rhs)
{
    return var.Get() + rhs;
}

inline std::string operator+(const Variable<std::string>& var, const char* rhs)
{
    return var.Get() + std::string(rhs);
}

template <typename T>
T operator-(const Variable<T>& var, const T& rhs)
{
    return var.Get() - rhs;
}

template <typename T>
bool operator==(const Variable<T>& var, const T& rhs)
{
    return var.Get() == rhs;
}

template <typename T>
bool operator!=(const Variable<T>& var, const T& rhs)
{
    return var.Get() != rhs;
}

template <typename T>
bool operator>(const Variable<T>& var, const T& rhs)
{
    return var.Get() > rhs;
}

template <typename T>
bool operator<(const Variable<T>& var, const T& rhs)
{
    return var.Get() < rhs;
}

template <typename T>
bool operator>=(const Variable<T>& var, const T& rhs)
{
    return var.Get() >= rhs;
}

template <typename T>
bool operator<=(const Variable<T>& var, const T& rhs)
{
    return var.Get() <= rhs;
}

template <typename Format>
class Section
{
public:
    template <typename F>
    using iface_type = SectionIface<F>;

    friend class SectionIface<Format>;

    Section(bool optional = false)
        : initialized_(false)
        , optional_(optional)
    {
    }

    Section(const Section<Format>& other)
        : initialized_(other.initialized_)
        , optional_(other.optional_)
    {
    }

    Section<Format>& operator=(const Section<Format>& other)
    {
        interfaces_.clear();
        initialized_ = other.initialized_;
        optional_ = other.optional_;
        return *this;
    }

    Section(Section<Format>&& other) = delete;
    Section<Format>& operator=(Section<Format>&& other) = delete;

    virtual ~Section() = default;

    virtual void Init(const std::string& parse_path) = 0;

    virtual bool Initialized() const { return initialized_; }

    virtual bool Optional() const { return optional_; }

protected:
    template <typename T>
    void Register(const std::string& parse_path, T* element)
    {
        using iface_type = typename T::template iface_type<Format>;
        interfaces_.emplace_back(std::make_unique<iface_type>(parse_path, element));
    }

private:
    std::vector<std::unique_ptr<Interface<Format>>> interfaces_;
    bool initialized_ = false;
    bool optional_ = false;
};

template <typename T>
class Vector: public std::vector<T>
{
public:
    template <typename F>
    using iface_type = VectorIface<T, F>;

    using std::vector<T>::vector;

    template <typename OtherT, typename std::enable_if<std::is_convertible<T, OtherT>::value>::type* = nullptr>
    operator std::vector<OtherT>() const
    {
        std::vector<OtherT> result;
        for (const T& i : *this) {
            result.push_back((OtherT)i);
        }
        return result;
    }

    bool Initialized() const { return true; }
};

} // namespace uconfig
