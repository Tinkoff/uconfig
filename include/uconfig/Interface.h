#pragma once

#include "Objects.h"

namespace uconfig {

class BaseFormat
{
public:
    static inline const std::string name = "[NO FORMAT]";
    using source_type = void;
    using dest_type = void;

    template <typename DestT>
    boost::optional<DestT> Parse(const source_type* source, const std::string& path)
    {
        throw std::logic_error("used BaseFormat::Parse<T>");
    }

    template <typename SrcT>
    void Emit(const std::string& path, const SrcT& source, dest_type* dest)
    {
        throw std::logic_error("used BaseFormat::Emit<T>");
    }

    virtual std::string VectorElementPath(const std::string& vector_path, std::size_t index) = 0;
};

template <typename Format>
class Interface
{
public:
    using format_type = Format;
    using source_type = typename format_type::source_type;
    using dest_type = typename format_type::dest_type;

    virtual ~Interface() = default;
    virtual bool Parse(format_type& parser, const source_type* source) = 0;
    virtual void Emit(format_type& printer, dest_type* dest) const = 0;
    virtual const std::string& Path() const = 0;
};

template <typename T, typename Format>
class VariableIface: public Interface<Format>
{
public:
    using format_type = typename Interface<Format>::format_type;
    using source_type = typename Interface<Format>::source_type;
    using dest_type = typename Interface<Format>::dest_type;

    VariableIface(const std::string& parse_path, Variable<T>* var)
        : variable_path_(parse_path)
        , variable_(var)
    {
        if (!variable_) {
            throw std::runtime_error("invalid variable pointer to parse");
        }
    }

    VariableIface(const VariableIface<T, Format>&) = default;
    VariableIface<T, Format>& operator=(const VariableIface<T, Format>&) = default;
    VariableIface(VariableIface<T, Format>&&) noexcept = default;
    VariableIface<T, Format>& operator=(VariableIface<T, Format>&&) noexcept = default;

    virtual ~VariableIface() = default;

    virtual bool Parse(format_type& parser, const source_type* source) override
    {
        boost::optional<T> result_opt = parser.template Parse<T>(source, variable_path_);

        if (!result_opt) {
            if (!variable_->Initialized()) {
                throw ParseError(Format::name + " " + variable_path_ + " is not set");
            }
            return false;
        }
        *variable_ = std::move(*result_opt);
        return true;
    }

    virtual void Emit(format_type& printer, dest_type* dest) const override
    {
        printer.Emit(variable_->Get(), variable_path_, dest);
    }

    virtual const std::string& Path() const override
    {
        return variable_path_;
    }

private:
    std::string variable_path_;
    Variable<T>* variable_;
};

template <typename Format>
class SectionIface: public Interface<Format>
{
public:
    using format_type = typename Interface<Format>::format_type;
    using source_type = typename Interface<Format>::source_type;
    using dest_type = typename Interface<Format>::dest_type;

    SectionIface(const std::string& parse_path, Section<Format>* sec)
        : section_path_(parse_path)
        , section_(sec)
    {
        if (!section_) {
            throw std::runtime_error("invalid section pointer to parse");
        }
    }

    SectionIface(const SectionIface<Format>&) = default;
    SectionIface<Format>& operator=(const SectionIface<Format>&) = default;
    SectionIface(SectionIface<Format>&&) noexcept = default;
    SectionIface<Format>& operator=(SectionIface<Format>&&) noexcept = default;

    virtual ~SectionIface() = default;

    virtual bool Parse(format_type& parser, const source_type* source) override
    {
        bool parsed_ = false;
        bool fullset = true;

        section_->interfaces_.clear();
        section_->Init(section_path_);
        for (auto& iface : section_->interfaces_) {
            try {
                // at least one of section's interfaces should be parsed
                parsed_ |= iface->Parse(parser, source);
            } catch (const ParseError&) {
                // Parse() will throw if variable is not set (i.e. boost::none)
                if (section_->Optional()) {
                    // if section is optional, it is not initialized
                    fullset = false;
                } else {
                    throw;
                }
            }
        }

        section_->initialized_ = fullset;
        return parsed_;
    }

    virtual void Emit(format_type& printer, dest_type* dest) const override
    {
        if (!section_->Initialized()) {
            return;
        }

        section_->interfaces_.clear();
        section_->Init(section_path_);
        for (const auto& iface : section_->interfaces_) {
            iface->Emit(printer, dest);
        }
    }

    virtual const std::string& Path() const override
    {
        return section_path_;
    }

private:
    std::string section_path_;
    Section<Format>* section_;
};

template <typename T, typename Format>
class VectorIface: public Interface<Format>
{
public:
    using format_type = typename Interface<Format>::format_type;
    using source_type = typename Interface<Format>::source_type;
    using dest_type = typename Interface<Format>::dest_type;

    VectorIface(const std::string& parse_path, Vector<T>* list)
        : vector_path_(parse_path)
        , element_vector_(list)
    {
        if (!element_vector_) {
            throw std::runtime_error("invalid list pointer to parse");
        }
    }

    VectorIface(const VectorIface<T, Format>&) = default;
    VectorIface<T, Format>& operator=(const VectorIface<T, Format>&) = default;
    VectorIface(VectorIface<T, Format>&&) noexcept = default;
    VectorIface<T, Format>& operator=(VectorIface<T, Format>&&) noexcept = default;

    virtual ~VectorIface() = default;

    virtual bool Parse(format_type& parser, const source_type* source) override
    {
        using iface_type = typename T::template iface_type<Format>;

        std::size_t index = 0;
        while (true) {
            T element;
            bool parsed = false;

            iface_type iface(parser.VectorElementPath(vector_path_, index), &element);
            try {
                parsed = iface.Parse(parser, source);
            } catch (const ParseError&) {
            }
            if (!parsed) {
                break;
            }

            ++index;
            element_vector_->emplace_back(std::move(element));
        }
        return index > 0;
    }

    virtual void Emit(format_type& printer, dest_type* dest) const override
    {
        using iface_type = typename T::template iface_type<Format>;

        for (std::size_t i = 0; i < element_vector_->size(); ++i) {
            iface_type iface(printer.VectorElementPath(vector_path_, i), &element_vector_->at(i));
            iface.Emit(printer, dest);
        }
    }

    virtual const std::string& Path() const override
    {
        return vector_path_;
    }

private:
    std::string vector_path_;
    Vector<T>* element_vector_;
};

} // namespace uconfig
