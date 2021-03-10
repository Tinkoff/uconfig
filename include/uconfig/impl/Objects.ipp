#pragma once

#include <ostream>

namespace uconfig {

template <typename... FormatTs>
Config<FormatTs...>::Config(bool optional)
    : optional_(optional)
{
}

template <typename... FormatTs>
Config<FormatTs...>::Config(const Config<FormatTs...>& other)
    : optional_(other.optional_)
{
}

template <typename... FormatTs>
Config<FormatTs...>& Config<FormatTs...>::operator=(const Config<FormatTs...>& other)
{
    if (this != &other) {
        Reset();
        optional_ = other.optional_;
    }
    return *this;
}

template <typename... FormatTs>
Config<FormatTs...>::Config(Config<FormatTs...>&& other) noexcept
    : optional_(std::move(other.optional_))
{
}

template <typename... FormatTs>
Config<FormatTs...>& Config<FormatTs...>::operator=(Config<FormatTs...>&& other) noexcept
{
    if (this != &other) {
        Reset();
        optional_ = std::move(other.optional_);
    }
    return *this;
}

template <typename... FormatTs>
template <typename F>
bool Config<FormatTs...>::Parse(const F& parser, const std::string& path, const typename F::source_type* source,
                                bool throw_on_fail)
{
    return iface_type<F>{path, this}.Parse(parser, source, throw_on_fail);
}

template <typename... FormatTs>
template <typename F>
void Config<FormatTs...>::Emit(const F& emitter, const std::string& path, typename F::dest_type* destination,
                               bool throw_on_fail)
{
    return iface_type<F>{path, this}.Emit(emitter, destination, throw_on_fail);
}

template <typename... FormatTs>
bool Config<FormatTs...>::Initialized() const noexcept
{
    for (const auto* elem : elements_) {
        if (!elem->Initialized() && !elem->Optional()) {
            return false;
        }
    }
    return true;
}

template <typename... FormatTs>
bool Config<FormatTs...>::Optional() const noexcept
{
    return optional_;
}

template <typename... FormatTs>
template <typename F, typename T>
void Config<FormatTs...>::Register(const std::string& element_path, T* element) noexcept
{
    using elem_iface_type = typename T::template iface_type<F>;

    if (!register_formats_.count(std::type_index(typeid(F)))) {
        return;
    }

    auto& fmt_ifaces = Interfaces<F>();
    fmt_ifaces.emplace_back(std::make_unique<elem_iface_type>(element_path, element));
    elements_.insert(element);
}

template <typename... FormatTs>
void Config<FormatTs...>::Reset() noexcept
{
    elements_ = {};
    interfaces_ = {};
    register_formats_ = {};
}

template <typename... FormatTs>
template <typename F>
void Config<FormatTs...>::SetFormat() noexcept
{
    register_formats_.insert(std::type_index(typeid(F)));
}

template <typename... FormatTs>
template <typename F>
std::vector<std::unique_ptr<Interface<F>>>& Config<FormatTs...>::Interfaces() noexcept
{
    return std::get<std::vector<std::unique_ptr<Interface<F>>>>(interfaces_);
}

template <typename T>
Variable<T>::Variable()
    : optional_(false)
    , value_(std::nullopt)
{
}

template <typename T>
Variable<T>::Variable(T&& init_value)
    : optional_(true)
    , value_(std::move(init_value))
{
}

template <typename T>
Variable<T>::Variable(const T& init_value)
    : optional_(true)
    , value_(init_value)
{
}

template <typename T>
Variable<T>& Variable<T>::operator=(T&& other) noexcept
{
    value_ = std::move(other);
    return *this;
}

template <typename T>
bool Variable<T>::Initialized() const noexcept
{
    return value_ != std::nullopt;
}

template <typename T>
bool Variable<T>::Optional() const noexcept
{
    return optional_;
}

template <typename T>
T& Variable<T>::Get()
{
    if (!Initialized()) {
        throw Error("failed to get variable value: it is not set");
    }
    return *value_;
}

template <typename T>
const T& Variable<T>::Get() const
{
    if (!Initialized()) {
        throw Error("failed to get variable value: it is not set");
    }
    return *value_;
}

template <typename T>
T& Variable<T>::operator*()
{
    return Get();
}

template <typename T>
const T& Variable<T>::operator*() const
{
    return Get();
}

template <typename T>
T* Variable<T>::operator->()
{
    return &Get();
}

template <typename T>
const T* Variable<T>::operator->() const
{
    return &Get();
}

template <typename T>
Variable<T>::operator T&()
{
    return Get();
}

template <typename T>
Variable<T>::operator const T&() const
{
    return Get();
}

template <typename T>
Vector<T>::Vector(bool optional)
{
    this->optional_ = optional;
    this->value_ = std::nullopt;
}

template <typename T>
Vector<T>::Vector(std::vector<T>&& init_value)
{
    this->optional_ = true;
    this->value_ = std::move(init_value);
}

template <typename T>
Vector<T>::Vector(const std::vector<T>& init_value)
{
    this->optional_ = true;
    this->value_ = init_value;
}

template <typename T>
Vector<T>& Vector<T>::operator=(std::vector<T>&& vector) noexcept
{
    this->value_ = std::move(vector);
    return *this;
}

template <typename T>
T& Vector<T>::operator[](std::size_t pos)
{
    return this->Get()[pos];
}

template <typename T>
const T& Vector<T>::operator[](std::size_t pos) const
{
    return this->Get()[pos];
}

/// If variable has value insert it into the stream, otherwise insert "[not set]".
template <typename V, std::enable_if_t<!detail::is_base_of_template<V, std::vector>::value, bool> = true>
std::ostream& operator<<(std::ostream& out, const Variable<V>& var)
{
    if (!var.Initialized()) {
        return out << "[not set]";
    }
    return out << var.Get();
}

/// operator+ for V and Variable<V>.
template <typename V>
V operator+(const V& lhs, const Variable<V>& rhs)
{
    return lhs + rhs.Get();
}
/// operator+ for Variable<V> and char*.
template <typename V>
V operator+(const Variable<V>& lhs, const V& rhs)
{
    return lhs.Get() + rhs;
}
/// operator+ for char* and Variable<std::string>.
inline std::string operator+(const char* lhs, const Variable<std::string>& rhs)
{
    return std::string(lhs) + rhs.Get();
}
/// operator+ for Variable<std::string> and char*.
inline std::string operator+(const Variable<std::string>& lhs, const char* rhs)
{
    return lhs.Get() + std::string(rhs);
}

/// operator- for V and Variable<V>.
template <typename V>
V operator-(const V& lhs, const Variable<V>& rhs)
{
    return lhs - rhs.Get();
}
/// operator- for Variable<V> and V.
template <typename V>
Variable<V> operator-(const Variable<V>& lhs, const V& rhs)
{
    return Variable<V>(lhs.Get() - rhs);
}

/// operator== for Variable<V> and Variable<U>.
template <typename V, typename U>
bool operator==(const Variable<V>& lhs, const Variable<U>& rhs)
{
    return lhs.value_ == rhs.value_;
}
/// operator== for V and Variable<V>.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
bool operator==(const U& lhs, const Variable<V>& rhs)
{
    return lhs == rhs.value_;
}
/// operator== for V and Variable<V>.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
bool operator==(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ == rhs;
}
/// operator== for Vector<Variable<V>> and std::vector<V>.
template <typename V>
bool operator==(const Vector<Variable<V>>& lhs, const std::vector<V>& rhs)
{
    if (!lhs.value_.has_value()) {
        return false;
    }
    if (lhs.value_->size() != rhs.size()) {
        return false;
    }
    for (std::size_t pos = 0; pos < rhs.size(); ++pos) {
        if (lhs.value_->at(pos) != rhs.at(pos)) {
            return false;
        }
    }
    return true;
}
/// operator== for std::vector<V> and Vector<Variable<V>>.
template <typename V>
bool operator==(const std::vector<V>& lhs, const Vector<Variable<V>>& rhs)
{
    return rhs == lhs;
}

/// operator!= for Variable<V> and Variable<U>.
template <typename V, typename U>
bool operator!=(const Variable<V>& lhs, const Variable<U>& rhs)
{
    return lhs.value_ != rhs.value_;
}
/// operator!= for V and Variable<V>.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
bool operator!=(const U& lhs, const Variable<V>& rhs)
{
    return lhs != rhs.value_;
}
/// operator!= for V and Variable<V>.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool>>
bool operator!=(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ != rhs;
}
/// operator!= for Vector<Variable<V>> and std::vector<V>.
template <typename V>
bool operator!=(const Vector<Variable<V>>& lhs, const std::vector<V>& rhs)
{
    return !(lhs == rhs);
}
/// operator== for std::vector<V> and Vector<Variable<V>>.
template <typename V>
bool operator!=(const std::vector<V>& lhs, const Vector<Variable<V>>& rhs)
{
    return !(lhs == rhs);
}

/// operator> for V and Variable<V>.
template <typename V, typename U>
bool operator>(const U& lhs, const Variable<V>& rhs)
{
    return lhs > rhs.value_;
}
/// operator> for V and Variable<V>.
template <typename V, typename U>
bool operator>(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ > rhs;
}

/// operator< for V and Variable<V>.
template <typename V, typename U>
bool operator<(const U& lhs, const Variable<V>& rhs)
{
    return lhs < rhs.value_;
}
/// operator< for V and Variable<V>.
template <typename V, typename U>
bool operator<(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ < rhs;
}

/// operator>= for V and Variable<V>.
template <typename V, typename U>
bool operator>=(const U& lhs, const Variable<V>& rhs)
{
    return lhs >= rhs.value_;
}
/// operator>= for V and Variable<V>.
template <typename V, typename U>
bool operator>=(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ >= rhs;
}

/// operator<= for V and Variable<V>.
template <typename V, typename U>
bool operator<=(const U& lhs, const Variable<V>& rhs)
{
    return lhs <= rhs.value_;
}
/// operator<= for V and Variable<V>.
template <typename V, typename U>
bool operator<=(const Variable<V>& lhs, const U& rhs)
{
    return lhs.value_ <= rhs;
}

} // namespace uconfig
