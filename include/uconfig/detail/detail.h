#pragma once

#include "forward.h"

#include <utility>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace uconfig {
namespace detail {

template <template <typename...> class C, typename... Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);

template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);

template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template <typename T>
struct enable_if_type
{
    typedef void type;
};

template <typename T, typename F, typename = void>
struct deduce_iface
{
    typedef ValueIface<T, F> type;
};

template <typename T, typename F>
struct deduce_iface<T, F, typename enable_if_type<typename T::template iface_type<F>>::type>
{
    typedef typename T::template iface_type<F> type;
};

template <typename T, typename F>
using deduce_iface_t = typename deduce_iface<T, F>::type;

} // namespace detail
} // namespace uconfig
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "forward_detail.h"
