#pragma once

#include "detail/detail.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <unordered_set>
#include <vector>

namespace uconfig {

/// Base error thrown in uconfig.
struct Error: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/// Parser error thrown in uconfig.
struct ParseError: public Error
{
    using Error::Error;
};

/// Emitter error thrown in uconfig.
struct EmitError: public Error
{
    using Error::Error;
};

/// Abstract object interface.
class Object
{
public:
    // template <typename F>
    // using iface_type;

    virtual ~Object() = default;

    virtual bool Initialized() const noexcept = 0;
    virtual bool Optional() const noexcept = 0;
    virtual void Validate() const {};
};

/**
 * Configuration object.
 *
 * @tparam FormatTs Types of the formatter to parse/emit.
 */
template <typename... FormatTs>
class Config: public Object
{
public:
    template <typename F>
    using iface_type = ConfigIface<F>;

    template <typename F>
    friend class ConfigIface;

    /**
     * Constructor.
     *
     * @param[in] optional If section considered to be optional (may be not initialized). Default false.
     */
    Config(bool optional = false);

    /// Copy constructor.
    Config(const Config<FormatTs...>& other);
    /// Copy assignment.
    Config<FormatTs...>& operator=(const Config<FormatTs...>& other);
    /// Move constructor.
    Config(Config<FormatTs...>&& other) noexcept;
    /// Move assignment.
    Config<FormatTs...>& operator=(Config<FormatTs...>&& other) noexcept;

    /// Destructor.
    virtual ~Config() = default;

    /**
     * Parse the config at @p path from @p source using @p parser.
     *
     * @tparam F Type of the parser to use. Should be one of FormatTs.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] path Path where the config resides in @p source.
     * @param[in] source Source to parse from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse all mandatory children.
     *  Default true.
     *
     * @returns true if config has been found in some form in the @p source, false otherwise.
     * @throws uconfig::ParseError Thrown if @p throw_on_fail.
     */
    template <typename F>
    bool Parse(const F& parser, const std::string& path, const typename F::source_type* source,
               bool throw_on_fail = true);

    /**
     * Emit the config at @p path to @p destination using @p emitter.
     *
     * @tparam F Type of the emitter to use. Should be one of FormatTs.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] path Path where the config should be in @p destination.
     * @param[in] destination Destination to emit into.
     * @param[in] throw_on_fail Will throw an uconfig::EmitError is failed to emit. Default true.
     *
     * @throws uconfig::EmitError Thrown if @p throw_on_fail.
     */
    template <typename F>
    void Emit(const F& emitter, const std::string& path, typename F::dest_type* destination, bool throw_on_fail = true);

    /**
     * Check if config has all mandatory values.
     *
     * @returns true if it has, false otherwise.
     */
    virtual bool Initialized() const noexcept override;

    /**
     * Check if config is marked as optional.
     *
     * @returns true if has been, false otherwise.
     */
    virtual bool Optional() const noexcept override;

protected:
    /**
     * Initialize config before parsing.
     * This function get called within Parse() and expected to register all variables
     *  in the config with respected paths. To register a variable call Register().
     *
     * @param[in] config_path Path of this config for the Format.
     */
    virtual void Init(const std::string& config_path) = 0;

    /**
     * Register a child for this config.
     * This function should be called for all children of the config to get them parsed/emitted.
     *
     * @tparam F Type of the format to register for.
     * @tparam T Type of element to register. Should be either uconfig::Variable,
     *  uconfig::Vector or derivative of uconfig::Config.
     *
     * @param[in] element_path Path of the child.
     * @param[in] element Pointer to the child.
     */
    template <typename F, typename T>
    void Register(const std::string& element_path, T* element) noexcept;

private:
    /// Reset all registered children.
    void Reset() noexcept;

    template <typename F>
    void SetFormat() noexcept;

    /// Get all registered children interfaces for specified format.
    template <typename F>
    std::vector<std::unique_ptr<Interface<F>>>& Interfaces() noexcept;

private:
    bool optional_ = false;
    std::unordered_set<Object*> elements_;
    std::unordered_set<std::type_index> register_formats_;
    std::tuple<std::vector<std::unique_ptr<Interface<FormatTs>>>...> interfaces_;
};

/**
 * Variable object.
 *
 * @tparam T Type of underlying variable to store.
 */
template <typename T>
class Variable: public Object
{
public:
    template <typename F>
    using iface_type = VariableIface<T, F>;

    template <typename U, typename F>
    friend class VariableIface;

    /// Constructor.
    Variable();
    /// Constructor.
    Variable(T&& init_value);
    /// Constructor.
    Variable(const T& init_value);

    /// Copy constructor.
    Variable(const Variable<T>&) = default;
    /// Copy assignment.
    Variable<T>& operator=(const Variable<T>&) = default;
    /// Move constructor.
    Variable(Variable<T>&& other) noexcept = default;
    /// Move assignment.
    Variable<T>& operator=(Variable<T>&& other) noexcept = default;
    /// Move assignment from T.
    Variable<T>& operator=(T&& other) noexcept;

    /// Destructor.
    virtual ~Variable() = default;

    /**
     * Check if variable has a value.
     *
     * @returns true if it has, false otherwise.
     */
    virtual bool Initialized() const noexcept override;

    /**
     * Check if variable is marked as optional.
     *
     * @returns true if has been, false otherwise.
     */
    virtual bool Optional() const noexcept override;

    /**
     * Read the value.
     *
     * @returns A reference to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    T& Get();

    /**
     * Read the value.
     *
     * @returns A const reference to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    const T& Get() const;

    /**
     * Dereference operator. Read the value.
     *
     * @returns A reference to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    T& operator*();

    /**
     * Dereference operator. Read the value.
     *
     * @returns A const reference to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    const T& operator*() const;

    /**
     * Structure dereference operator. Read the value.
     *
     * @returns A pointer to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    T* operator->();

    /**
     * Structure dereference operator. Read the value.
     *
     * @returns A const pointer to the value.
     * @throws uconfig::Error Thrown if variable has no value.
     */
    const T* operator->() const;

    /**
     * Explicit conversation to `T&`.
     *
     * @throws uconfig::Error Thrown if variable has no value.
     */
    explicit operator T&();

    /**
     * Explicit conversation to `const T&`.
     *
     * @throws uconfig::Error Thrown if variable has no value.
     */
    explicit operator const T&() const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    /* enable left and right-handed comparisons */

    template <typename V, typename U>
    friend bool operator==(const Variable<V>& lhs, const Variable<U>& rhs);
    template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
    friend bool operator==(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
    friend bool operator==(const Variable<V>& lhs, const U& rhs);

    template <typename V, typename U>
    friend bool operator!=(const Variable<V>& lhs, const Variable<U>& rhs);
    template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
    friend bool operator!=(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
    friend bool operator!=(const Variable<V>& lhs, const U& rhs);

    template <typename V, typename U>
    friend bool operator>(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U>
    friend bool operator>(const Variable<V>& lhs, const U& rhs);

    template <typename V, typename U>
    friend bool operator<(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U>
    friend bool operator<(const Variable<V>& lhs, const U& rhs);

    template <typename V, typename U>
    friend bool operator>=(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U>
    friend bool operator>=(const Variable<V>& lhs, const U& rhs);

    template <typename V, typename U>
    friend bool operator<=(const U& lhs, const Variable<V>& rhs);
    template <typename V, typename U>
    friend bool operator<=(const Variable<V>& lhs, const U& rhs);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

protected:
    bool optional_ = false;
    std::optional<T> value_ = std::nullopt; ///< Stored value or none.
};

/**
 * Vector object.
 *
 * @tparam T Type to form vector of.
 */
template <typename T>
class Vector: public Variable<std::vector<T>>
{
public:
    template <typename F>
    using iface_type = VectorIface<T, F>;

    template <typename U, typename F>
    friend class VectorIface;

    /**
     * Constructor.
     *
     * @param[in] optional If vector considered to be optional (may be not initialized). Default false.
     */
    Vector(bool optional = false);
    /// Constructor.
    Vector(std::vector<T>&& init_value);
    /// Constructor.
    Vector(const std::vector<T>& init_value);

    /// Copy constructor.
    Vector(const Vector<T>&) = default;
    /// Copy assignment.
    Vector<T>& operator=(const Vector<T>&) = default;
    /// Move constructor.
    Vector(Vector<T>&& other) noexcept = default;
    /// Move assignment.
    Vector<T>& operator=(Vector<T>&& other) noexcept = default;
    /// Move assignment from std::vector<T>.
    Vector<T>& operator=(std::vector<T>&& vector) noexcept;

    /// Destructor.
    virtual ~Vector() = default;

    /**
     * Get the value from underlying vector.
     *
     * @param[in] pos Position to get value at.
     *
     * @returns A reference to the value at @p pos.
     * @throws uconfig::Error Thrown if vector has no value.
     */
    T& operator[](std::size_t pos);

    /**
     * Get the value from underlying vector.
     *
     * @param[in] pos Position to get value at.
     *
     * @returns A const reference to the value at @p pos.
     * @throws uconfig::Error Thrown if vector has no value.
     */
    const T& operator[](std::size_t pos) const;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

    // operator== for Vector<Variable<V>> and std::vector<V>.
    template <typename V>
    friend bool operator==(const Vector<Variable<V>>& lhs, const std::vector<V>& rhs);

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
};

} // namespace uconfig

#include "impl/Objects.ipp"
