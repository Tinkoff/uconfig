#pragma once

#include <boost/optional.hpp>

#include <list>
#include <memory>
#include <ostream>
#include <vector>

namespace uconfig {
/// Forward-declared Interface.
template <typename Format>
class Interface;
/// Forward-declared VariableIface.
template <typename T, typename Format>
class VariableIface;
/// Forward-declared SectionIface.
template <typename Format>
class SectionIface;
/// Forward-declared VectorIface.
template <typename T, typename Format>
class VectorIface;

struct ParseError: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/**
 * Configuration variable.
 *
 * This class implements storage entity for configuration variables.
 * Any config represented as a bunch of such variables grouped into
 * @p Section or @p Vector to provide nested structure.
 *
 * @tparam T Type of underlying variable to store.
 */
template <typename T>
class Variable
{
public:
    template <typename F>
    using iface_type = VariableIface<T, F>;

    /// Default constructor.
    Variable()
        : value_(boost::none)
    {
    }

    /**
     * Default-initialized variable constructor.
     *
     * @param[in] init_value Value to initialize variable with.
     */
    Variable(T&& init_value)
        : value_(std::move(init_value))
    {
    }

    /// Copy constructor.
    Variable(const Variable<T>&) = default;
    /// Copy assignment.
    Variable<T>& operator=(const Variable<T>&) = default;
    /// Move constructor.
    Variable(Variable<T>&& other) noexcept
        : value_(std::move(other.value_))
    {
    }
    /// Move assignment.
    Variable<T>& operator=(Variable<T>&& other) noexcept
    {
        value_ = std::move(other.value_);
        return *this;
    }
    /// Move assignment from rvalue of T.
    Variable<T>& operator=(T&& other) noexcept
    {
        value_ = std::move(other);
        return *this;
    }

    /// Destructor.
    virtual ~Variable() = default;

    /**
     * Read the value.
     *
     * @returns A const reference to the value.
     * @throws std::runtime_error Thrown if variable has no value.
     */
    const T& Get() const
    {
        if (!Initialized()) {
            throw std::runtime_error("config variable is not set");
        }
        return *value_;
    }

    /**
     * Check if variable has a value.
     *
     * @returns true if has, false otherwise.
     */
    bool Initialized() const
    {
        return value_ != boost::none;
    }

    operator const T&() const
    {
        return Get();
    }

    /**
     * Dereference operator. Access the value.
     *
     * @returns A const reference to the value.
     * @throws std::runtime_error Thrown if variable has no value.
     */
    const T& operator*() const
    {
        return Get();
    }

    /**
     * Structure dereference operator. Access the value.
     *
     * @returns A const pointer to the value.
     * @throws std::runtime_error Thrown if variable has no value.
     */
    const T* operator->() const
    {
        return &Get();
    }

protected:
    boost::optional<T> value_; ///< Stored value or none.
};

/// If variable has value insert it into the stream, otherwise insert "[not set]".
template <typename T>
std::ostream& operator<<(std::ostream& out, const Variable<T>& var)
{
    if (!var.Initialized()) {
        return out << "[not set]";
    }
    return out << var.Get();
}
/// operator+ for T and Variable<T>.
template <typename T>
T operator+(const T& lhs, const Variable<T>& var)
{
    return lhs + var.Get();
}
/// operator+ for char* and Variable<std::string>.
inline std::string operator+(const char* lhs, const Variable<std::string>& var)
{
    return std::string(lhs) + var.Get();
}
/// operator- for T and Variable<T>.
template <typename T>
T operator-(const T& lhs, const Variable<T>& var)
{
    return lhs - var.Get();
}
/// operator== for T and Variable<T>.
template <typename T>
bool operator==(const T& lhs, const Variable<T>& var)
{
    return lhs == var.Get();
}
/// operator!= for T and Variable<T>.
template <typename T>
bool operator!=(const T& lhs, const Variable<T>& var)
{
    return lhs != var.Get();
}
/// operator> for T and Variable<T>.
template <typename T>
bool operator>(const T& lhs, const Variable<T>& var)
{
    return lhs > var.Get();
}
/// operator< for T and Variable<T>.
template <typename T>
bool operator<(const T& lhs, const Variable<T>& var)
{
    return lhs < var.Get();
}
/// operator>= for T and Variable<T>.
template <typename T>
bool operator>=(const T& lhs, const Variable<T>& var)
{
    return lhs >= var.Get();
}
/// operator<= for T and Variable<T>.
template <typename T>
bool operator<=(const T& lhs, const Variable<T>& var)
{
    return lhs <= var.Get();
}
/// operator+ for Variable<T> and char*.
template <typename T>
T operator+(const Variable<T>& var, const T& rhs)
{
    return var.Get() + rhs;
}
/// operator+ for Variable<std::string> and char*.
inline std::string operator+(const Variable<std::string>& var, const char* rhs)
{
    return var.Get() + std::string(rhs);
}
/// operator- for T and Variable<T>.
template <typename T>
T operator-(const Variable<T>& var, const T& rhs)
{
    return var.Get() - rhs;
}
/// operator== for T and Variable<T>.
template <typename T>
bool operator==(const Variable<T>& var, const T& rhs)
{
    return var.Get() == rhs;
}
/// operator!= for T and Variable<T>.
template <typename T>
bool operator!=(const Variable<T>& var, const T& rhs)
{
    return var.Get() != rhs;
}
/// operator> for T and Variable<T>.
template <typename T>
bool operator>(const Variable<T>& var, const T& rhs)
{
    return var.Get() > rhs;
}
/// operator< for T and Variable<T>.
template <typename T>
bool operator<(const Variable<T>& var, const T& rhs)
{
    return var.Get() < rhs;
}
/// operator>= for T and Variable<T>.
template <typename T>
bool operator>=(const Variable<T>& var, const T& rhs)
{
    return var.Get() >= rhs;
}
/// operator<= for T and Variable<T>.
template <typename T>
bool operator<=(const Variable<T>& var, const T& rhs)
{
    return var.Get() <= rhs;
}

/**
 * Static container (group) of configuration parameters
 *
 * This class implements base for config sections. Inherited classes should register its' elements
 * (variables, sections, vectors) with Register() calls to enable parsing/emitting for the section.
 *
 * @tparam Format Type of the formatter to parse/emit.
 */
template <typename Format>
class Section
{
public:
    template <typename F>
    using iface_type = SectionIface<F>;

    friend class SectionIface<Format>;

    /**
     * Constructor.
     *
     * @param[in] optional If section considered to be optional (may be not initialized).
     */
    Section(bool optional = false)
        : initialized_(false)
        , optional_(optional)
    {
    }

    /**
     * Copy constructor.
     * Copy-initialized Section is not parsable. Call Init() first.
     */
    Section(const Section<Format>& other)
        : initialized_(other.initialized_)
        , optional_(other.optional_)
    {
    }

    /**
     * Copy assignment.
     * Copy-initialized Section is not parsable. Call Init() first.
     */
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

    virtual bool Initialized() const
    {
        return initialized_;
    }

    virtual bool Optional() const
    {
        return optional_;
    }

    virtual void PostParse() const {}

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

    bool Initialized() const
    {
        return true;
    }

    virtual void PostParse() const {}
};

} // namespace uconfig
