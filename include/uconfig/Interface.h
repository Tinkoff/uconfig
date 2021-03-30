#pragma once

#include "Objects.h"

#include <functional>

namespace uconfig {

/**
 * Abstract interface for the @p Format.
 * Separates uconfig::Format from uconfig::Object instances.
 * Derived instances wraps different objects and used to parse/emit them using @p Format.
 *
 * @tparam Format Format this interface interacts with.
 */
template <typename Format>
class Interface
{
public:
    /// Alias to the @p Format.
    typedef Format format_type;
    /// Source this interface parses from.
    typedef typename Format::source_type source_type;
    /// Destination this interface emits to.
    typedef typename Format::dest_type dest_type;

    /// Destructor.
    virtual ~Interface() = default;

    /**
     * Parse the @p source using @p parser.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] source Source to parse value from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse. Default true.
     *
     * @returns true if something has been parsed, false otherwise.
     * @throws uconfig::ParseError May be thrown if @p throw_on_fail.
     */
    virtual bool Parse(const format_type& parser, const source_type* source, bool throw_on_fail) = 0;

    /**
     * Emit to the @p destination using @p emitter.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] dest Destination to emit into.
     * @param[in] throw_on_fail Will throw an uconfig::EmitError is failed to emit. Default true.
     *
     * @throws uconfig::EmitError May be thrown if @p throw_on_fail.
     */
    virtual void Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail) = 0;

    /// Get path of the object according to the @p Format.
    virtual const std::string& Path() const noexcept = 0;
    /// Check if wrapped object has any value in it.
    virtual bool Initialized() const noexcept = 0;
    /// Check if wrapped object declared as optional.
    virtual bool Optional() const noexcept = 0;
};

/**
 * Interface for uconfig::Config objects.
 *
 * @tparam Format Format this interface interacts with.
 */
template <typename Format>
class ConfigIface: public Interface<Format>
{
public:
    /// Alias to the @p Format.
    using typename Interface<Format>::format_type;
    /// Alias to the @p Format::source_type.
    using typename Interface<Format>::source_type;
    /// Alias to the @p Format::dest_type.
    using typename Interface<Format>::dest_type;

    /**
     * Constructor.
     *
     * @tparam FormatTs Type of the formats for @p config. Should include @p Format.
     *
     * @param[in] parse_path Path to the config in terms of @p Format.
     * @param[in] config Pointer to the uconfig::Config to wrap.
     *
     * @note Does not own @p config, should not outlive it.
     */
    template <typename... FormatTs>
    ConfigIface(const std::string& parse_path, Config<FormatTs...>* config);

    /// Copy constructor.
    ConfigIface(const ConfigIface<Format>&) = default;
    /// Copy assignment.
    ConfigIface<Format>& operator=(const ConfigIface<Format>&) = default;
    /// Move constructor.
    ConfigIface(ConfigIface<Format>&&) noexcept = default;
    /// Move assignment.
    ConfigIface<Format>& operator=(ConfigIface<Format>&&) noexcept = default;

    /// Destructor.
    virtual ~ConfigIface() = default;

    /**
     * Parse referenced uconfig::Config from @p source using @p parser.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] source Source to parse from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse all mandatory values. Default true.
     *
     * @returns true if config has been parsed, false otherwise.
     * @throws uconfig::ParseError Thrown if @p throw_on_fail.
     */
    virtual bool Parse(const format_type& parser, const source_type* source, bool throw_on_fail = true) override;

    /**
     * Emit referenced uconfig::Config to @p destination using @p emitter.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] dest Destination to emit into.
     * @param[in] throw_on_fail Will throw an uconfig::EmitError is failed to emit. Default true.
     *
     * @throws uconfig::EmitError Thrown if @p throw_on_fail.
     */
    virtual void Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail = true) override;

    /// Get path of the wrapped uconfig::Config.
    virtual const std::string& Path() const noexcept override;
    /// Check if wrapped uconfig::Config has all mandatory values set.
    virtual bool Initialized() const noexcept override;
    /// Check if wrapped uconfig::Config declared as optional.
    virtual bool Optional() const noexcept override;

private:
    std::string path_;
    bool cfg_optional_;
    std::vector<std::unique_ptr<Interface<format_type>>>* cfg_interfaces_;
    std::function<void()> cfg_validate_;
};

/**
 * Interface for raw values. Used to interface uconfig::Vector elements.
 *
 * @tparam T Type of the value.
 * @tparam Format Format this interface interacts with.
 */
template <typename T, typename Format>
class ValueIface: public Interface<Format>
{
public:
    /// Alias to the @p Format.
    using typename Interface<Format>::format_type;
    /// Alias to the @p Format::source_type.
    using typename Interface<Format>::source_type;
    /// Alias to the @p Format::dest_type.
    using typename Interface<Format>::dest_type;

    /**
     * Constructor.
     *
     * @param[in] variable_path Path to the variable in terms of @p Format.
     * @param[in] value Pointer to the value to wrap.
     *
     * @note Does not own @p value, should not outlive it.
     */
    ValueIface(const std::string& variable_path, T* value);

    /// Copy constructor.
    ValueIface(const ValueIface<T, Format>&) = default;
    /// Copy assignment.
    ValueIface<T, Format>& operator=(const ValueIface<T, Format>&) = default;
    /// Move constructor.
    ValueIface(ValueIface<T, Format>&&) noexcept = default;
    /// Move assignment.
    ValueIface<T, Format>& operator=(ValueIface<T, Format>&&) noexcept = default;

    /// Destructor.
    virtual ~ValueIface() = default;

    /**
     * Parse referenced value from @p source using @p parser.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] source Source to parse from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse. Default true.
     *
     * @returns true if value has been parsed, false otherwise.
     * @throws uconfig::ParseError Thrown if @p throw_on_fail.
     */
    virtual bool Parse(const format_type& parser, const source_type* source, bool throw_on_fail = true) override;

    /**
     * Emit referenced value to @p destination using @p emitter.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] dest Destination to emit into.
     * @param[in] throw_on_fail Not used. Always throws if failed to emit.
     *
     * @throws uconfig::EmitError Thrown if failed.
     */
    virtual void Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail = true) override;

    /// Get path of the wrapped value.
    virtual const std::string& Path() const noexcept override;
    /// Check if wrapped value has all mandatory values set.
    virtual bool Initialized() const noexcept override;
    /// Check if wrapped value declared as optional.
    virtual bool Optional() const noexcept override;

private:
    std::string path_;
    bool initialized_;
    T* value_ptr_;
};

/**
 * Interface for uconfig::Variable objects.
 *
 * @tparam T Variable wrapped type.
 * @tparam Format Format this interface interacts with.
 */
template <typename T, typename Format>
class VariableIface: public Interface<Format>
{
public:
    /// Alias to the @p Format.
    using typename Interface<Format>::format_type;
    /// Alias to the @p Format::source_type.
    using typename Interface<Format>::source_type;
    /// Alias to the @p Format::dest_type.
    using typename Interface<Format>::dest_type;

    /**
     * Constructor.
     *
     * @param[in] variable_path Path to the variable in terms of @p Format.
     * @param[in] variable Pointer to the uconfig::Variable<> to wrap.
     *
     * @note Does not own @p variable, should not outlive it.
     */
    VariableIface(const std::string& variable_path, Variable<T>* variable);

    /// Copy constructor.
    VariableIface(const VariableIface<T, Format>&) = default;
    /// Copy assignment.
    VariableIface<T, Format>& operator=(const VariableIface<T, Format>&) = default;
    /// Move constructor.
    VariableIface(VariableIface<T, Format>&&) noexcept = default;
    /// Move assignment.
    VariableIface<T, Format>& operator=(VariableIface<T, Format>&&) noexcept = default;

    /// Destructor.
    virtual ~VariableIface() = default;

    /**
     * Parse referenced uconfig::Variable<> from @p source using @p parser.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] source Source to parse from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse. Default true.
     *
     * @returns true if variable has been parsed, false otherwise.
     * @throws uconfig::ParseError Thrown if @p throw_on_fail.
     */
    virtual bool Parse(const format_type& parser, const source_type* source, bool throw_on_fail = true) override;

    /**
     * Emit referenced uconfig::Variable<> to @p destination using @p emitter.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] dest Destination to emit into.
     * @param[in] throw_on_fail Will throw an uconfig::EmitError is failed to emit. Default true.
     *
     * @throws uconfig::EmitError Thrown if @p throw_on_fail.
     */
    virtual void Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail = true) override;

    /// Get path of the wrapped uconfig::Variable<>.
    virtual const std::string& Path() const noexcept override;
    /// Check if wrapped uconfig::Variable<> has all mandatory values set.
    virtual bool Initialized() const noexcept override;
    /// Check if wrapped uconfig::Variable<> declared as optional.
    virtual bool Optional() const noexcept override;

private:
    std::string path_;
    Variable<T>* variable_ptr_;
};

/**
 * Interface for uconfig::Vector objects.
 *
 * @tparam T Type of Vector elements.
 * @tparam Format Format this interface interacts with.
 */
template <typename T, typename Format>
class VectorIface: public Interface<Format>
{
public:
    /// Alias to the @p Format.
    using typename Interface<Format>::format_type;
    /// Alias to the @p Format::source_type.
    using typename Interface<Format>::source_type;
    /// Alias to the @p Format::dest_type.
    using typename Interface<Format>::dest_type;

    /**
     * Constructor.
     *
     * @param[in] vector_path Path to the vector in terms of @p Format.
     * @param[in] vector Pointer to the uconfig::Vector to wrap.
     *
     * @note Does not own @p vector, should not outlive it.
     */
    VectorIface(const std::string& vector_path, Vector<T>* vector);

    /// Copy constructor.
    VectorIface(const VectorIface<T, Format>&) = default;
    /// Copy assignment.
    VectorIface<T, Format>& operator=(const VectorIface<T, Format>&) = default;
    /// Move constructor.
    VectorIface(VectorIface<T, Format>&&) noexcept = default;
    /// Move assignment.
    VectorIface<T, Format>& operator=(VectorIface<T, Format>&&) noexcept = default;

    /// Destructor.
    virtual ~VectorIface() = default;

    /**
     * Parse referenced uconfig::Vector from @p source using @p parser.
     *
     * @param[in] parser Parser instance to use.
     * @param[in] source Source to parse from.
     * @param[in] throw_on_fail Will throw an uconfig::ParseError is failed to parse. Default true.
     *
     * @returns true if vector has been parsed, false otherwise.
     * @throws uconfig::ParseError Thrown if @p throw_on_fail.
     */
    virtual bool Parse(const format_type& parser, const source_type* source, bool throw_on_fail = true) override;

    /**
     * Emit referenced uconfig::Vector to @p destination using @p emitter.
     *
     * @param[in] emitter Emitter instance to use.
     * @param[in] dest Destination to emit into.
     * @param[in] throw_on_fail Will throw an uconfig::EmitError is failed to emit. Default true.
     *
     * @throws uconfig::EmitError Thrown if @p throw_on_fail.
     */
    virtual void Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail = true) override;

    /// Get path of the wrapped uconfig::Vector.
    virtual const std::string& Path() const noexcept override;
    /// Check if wrapped uconfig::Vector has all mandatory values set.
    virtual bool Initialized() const noexcept override;
    /// Check if wrapped uconfig::Vector declared as optional.
    virtual bool Optional() const noexcept override;

private:
    std::string path_;
    Vector<T>* vector_ptr_;
};

} // namespace uconfig

#include "impl/Interface.ipp"
