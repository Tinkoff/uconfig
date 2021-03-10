#pragma once

namespace uconfig {

/// Forward-declared Interface.
template <typename Format>
class Interface;
/// Forward-declared ConfigIface.
template <typename Format>
class ConfigIface;
/// Forward-declared VariableIface.
template <typename T, typename Format>
class VariableIface;
/// Forward-declared ValueIface.
template <typename T, typename Format>
class ValueIface;
/// Forward-declared VectorIface.
template <typename T, typename Format>
class VectorIface;

/// Forward-declared Config.
template <typename ...FormatTs>
class Config;
/// Forward-declared Variable.
template <typename T>
class Variable;
/// Forward-declared Vector.
template <typename T>
class Vector;

} // namespace uconfig
