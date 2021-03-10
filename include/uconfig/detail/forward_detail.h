#pragma once

#include <type_traits>

namespace uconfig {

/// Forward-declared operator==.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool> = true>
bool operator==(const U& lhs, const Variable<V>& rhs);
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool> = true>
bool operator==(const Variable<V>& lhs, const U& rhs);
/// Forward-declared operator!=.
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool> = true>
bool operator!=(const U& lhs, const Variable<V>& rhs);
template <typename V, typename U, std::enable_if_t<!detail::is_base_of_template<U, Variable>::value, bool> = true>
bool operator!=(const Variable<V>& lhs, const U& rhs);

} // namespace uconfig
