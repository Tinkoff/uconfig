#pragma once

namespace uconfig {

template <typename Format>
template <typename... FormatTs>
ConfigIface<Format>::ConfigIface(const std::string& parse_path, Config<FormatTs...>* config)
    : path_(parse_path)
{
    if (!config) {
        throw std::runtime_error("invalid section pointer to parse");
    }
    config->Reset();
    config->template SetFormat<Format>();
    config->Init(Path());
    cfg_optional_ = config->Optional();
    cfg_interfaces_ = &config->template Interfaces<format_type>();
    cfg_validate_ = [config]() { config->Validate(); };
}

template <typename Format>
bool ConfigIface<Format>::Parse(const format_type& parser, const source_type* source, bool throw_on_fail)
{
    bool config_parsed = false;

    for (auto& iface : *cfg_interfaces_) {
        bool iface_parsed;
        try {
            iface_parsed = iface->Parse(parser, source, throw_on_fail);
        } catch (const Error& ex) {
            iface_parsed = false;
            if (!Optional() && throw_on_fail) {
                throw ParseError(ex.what());
            }
        }
        // section considered parsed if at least one of its' interfaces parsed
        config_parsed |= iface_parsed;
    }

    try {
        cfg_validate_();
    } catch (const Error& ex) {
        if (throw_on_fail) {
            throw ParseError(ex.what());
        }
    } catch (const std::exception& ex) {
        if (throw_on_fail) {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: " + ex.what());
        }
    }
    return config_parsed;
}

template <typename Format>
void ConfigIface<Format>::Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail)
{
    for (auto& iface : *cfg_interfaces_) {
        try {
            iface->Emit(emitter, dest, throw_on_fail);
        } catch (const Error& ex) {
            if (!Optional() && throw_on_fail) {
                throw EmitError(ex.what());
            }
        }
    }
}

template <typename Format>
const std::string& ConfigIface<Format>::Path() const noexcept
{
    return path_;
}

template <typename Format>
bool ConfigIface<Format>::Initialized() const noexcept
{
    for (const auto& iface : *cfg_interfaces_) {
        if (!iface->Initialized() && !iface->Optional()) {
            return false;
        }
    }
    return true;
}

template <typename Format>
bool ConfigIface<Format>::Optional() const noexcept
{
    return cfg_optional_;
}

template <typename T, typename Format>
ValueIface<T, Format>::ValueIface(const std::string& variable_path, T* value)
    : path_(variable_path)
    , initialized_(false)
    , value_ptr_(value)
{
    if (!value_ptr_) {
        throw std::runtime_error("invalid variable pointer to parse");
    }
}

template <typename T, typename Format>
bool ValueIface<T, Format>::Parse(const format_type& parser, const source_type* source, bool throw_on_fail)
{
    std::optional<T> result_opt = parser.template Parse<T>(source, Path());

    if (!result_opt) {
        if (!Optional() && throw_on_fail) {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: variable is not set");
        }
        return false;
    }
    *value_ptr_ = std::move(*result_opt);
    initialized_ = true;
    return true;
}

template <typename T, typename Format>
void ValueIface<T, Format>::Emit(const format_type& emitter, dest_type* dest, bool /*throw_on_fail*/)
{
    emitter.Emit(dest, Path(), *value_ptr_);
}

template <typename T, typename Format>
const std::string& ValueIface<T, Format>::Path() const noexcept
{
    return path_;
}

template <typename T, typename Format>
bool ValueIface<T, Format>::Initialized() const noexcept
{
    return initialized_;
}

template <typename T, typename Format>
bool ValueIface<T, Format>::Optional() const noexcept
{
    return false;
}

template <typename T, typename Format>
VariableIface<T, Format>::VariableIface(const std::string& variable_path, Variable<T>* variable)
    : path_(variable_path)
    , variable_ptr_(variable)
{
    if (!variable_ptr_) {
        throw std::runtime_error("invalid variable pointer to parse");
    }
}

template <typename T, typename Format>
bool VariableIface<T, Format>::Parse(const format_type& parser, const source_type* source, bool throw_on_fail)
{
    std::optional<T> result_opt = parser.template Parse<T>(source, Path());

    if (!result_opt) {
        if (!Initialized() && throw_on_fail) {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: variable is not set");
        }
        return false;
    }

    *variable_ptr_ = std::move(*result_opt);
    try {
        variable_ptr_->Validate();
    } catch (const Error& ex) {
        if (throw_on_fail) {
            throw ParseError(ex.what());
        }
    } catch (const std::exception& ex) {
        if (throw_on_fail) {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: " + ex.what());
        }
    }
    return true;
}

template <typename T, typename Format>
void VariableIface<T, Format>::Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail)
{
    try {
        emitter.Emit(dest, Path(), variable_ptr_->Get());
    } catch (const std::exception& ex) {
        if (throw_on_fail) {
            throw EmitError(format_type::name + " config '" + Path() + "' is not valid: " + ex.what());
        }
        return;
    }
}

template <typename T, typename Format>
const std::string& VariableIface<T, Format>::Path() const noexcept
{
    return path_;
}

template <typename T, typename Format>
bool VariableIface<T, Format>::Initialized() const noexcept
{
    return variable_ptr_->Initialized();
}

template <typename T, typename Format>
bool VariableIface<T, Format>::Optional() const noexcept
{
    return variable_ptr_->Optional();
}

template <typename T, typename Format>
VectorIface<T, Format>::VectorIface(const std::string& vector_path, Vector<T>* list)
    : path_(vector_path)
    , vector_ptr_(list)
{
    if (!vector_ptr_) {
        throw std::runtime_error("invalid list pointer to parse");
    }
}

template <typename T, typename Format>
bool VectorIface<T, Format>::Parse(const format_type& parser, const source_type* source, bool throw_on_fail)
{
    using elem_iface_type = detail::deduce_iface_t<T, Format>;

    std::size_t index = 0;
    std::optional<Error> last_error;
    while (true) {
        T element;

        elem_iface_type iface(parser.VectorElementPath(Path(), index), &element);
        try {
            iface.Parse(parser, source, true);
        } catch (const Error& ex) {
            // always stop on fail to prevent looping
            last_error = ex;
            break;
        }

        // we are about to emplace first parsed value, vector should be empty-initialized to do so
        if (!Initialized() || index == 0) {
            vector_ptr_->value_ = std::vector<T>();
        }
        vector_ptr_->value_->emplace_back(std::move(element));
        ++index;
    }

    if (!Initialized() && !Optional()) {
        // notify that mandatory vector was not parsed
        if (last_error) {
            throw ParseError(last_error->what());
        } else {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: vector is not set");
        }
    }

    try {
        vector_ptr_->Validate();
    } catch (const Error& ex) {
        if (throw_on_fail) {
            throw ParseError(ex.what());
        }
    } catch (const std::exception& ex) {
        if (throw_on_fail) {
            throw ParseError(format_type::name + " config '" + Path() + "' is not valid: " + ex.what());
        }
    }
    return index > 0;
}

template <typename T, typename Format>
void VectorIface<T, Format>::Emit(const format_type& emitter, dest_type* dest, bool throw_on_fail)
{
    using elem_iface_type = detail::deduce_iface_t<T, Format>;

    std::size_t index = 0;
    while (true) {
        T* elem = nullptr;
        std::string elem_path = emitter.VectorElementPath(Path(), index);

        try {
            elem = &(*vector_ptr_)->at(index);
        } catch (const Error& ex) {
            if (!Optional() && throw_on_fail) {
                throw EmitError(format_type::name + " config '" + elem_path + "' is not valid: " + ex.what());
            }
            return;
        } catch (const std::out_of_range& ex) {
            if (!Optional() && throw_on_fail) {
                throw EmitError(format_type::name + " config '" + elem_path + "' is not valid: variable is not set");
            }
            return;
        }

        elem_iface_type iface(elem_path, elem);
        iface.Emit(emitter, dest, throw_on_fail);

        if (++index >= (*vector_ptr_)->size()) {
            return;
        }
    }
}

template <typename T, typename Format>
const std::string& VectorIface<T, Format>::Path() const noexcept
{
    return path_;
}

template <typename T, typename Format>
bool VectorIface<T, Format>::Initialized() const noexcept
{
    return vector_ptr_->Initialized();
}

template <typename T, typename Format>
bool VectorIface<T, Format>::Optional() const noexcept
{
    return vector_ptr_->Optional();
}

} // namespace uconfig
