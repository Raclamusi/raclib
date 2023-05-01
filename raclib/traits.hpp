#pragma once

#include <type_traits>
#include <tuple>
#include <iterator>
#include <utility>

namespace raclib {
    namespace detail {
        using std::begin, std::end, std::size;

        template <class T, class = void>
        struct is_tuple_like_impl : std::false_type {};
        template <class T>
        struct is_tuple_like_impl<T, std::void_t<typename std::tuple_size<T>::type>> : std::true_type {};

        template <class T, class = void>
        struct is_range_impl : std::false_type {};
        template <class T>
        struct is_range_impl<T, decltype(begin(std::declval<T&>()), end(std::declval<T&>()), void())> : std::true_type {};

        template <class T>
        using iterator_type_impl = decltype(begin(std::declval<T&>()));
        template <class T>
        using range_size_type_impl = decltype(size(std::declval<T&>()));

        template <class T, class... Args, decltype(T{ std::declval<Args>()... }, 0) = 0>
        std::true_type is_curly_bracket_constructible_impl(int);
        template <class T, class... Args>
        std::false_type is_curly_bracket_constructible_impl(long);
    }

    template <class T>
    struct is_tuple_like : detail::is_tuple_like_impl<T>::type {};
    template <class T>
    inline constexpr bool is_tuple_like_v = is_tuple_like<T>::value;

    template <class T>
    struct is_range : detail::is_range_impl<T>::type {};
    template <class T>
    inline constexpr bool is_range_v = is_range<T>::value;

    template <class T>
    using iterator_type = detail::iterator_type_impl<T>;
    template <class T>
    using range_size_type = detail::range_size_type_impl<T>;
    template <class T>
    using range_diference_type = typename std::iterator_traits<std::decay_t<iterator_type<T>>>::difference_type;
    template <class T>
    using range_value_type = typename std::iterator_traits<std::decay_t<iterator_type<T>>>::value_type;
    template <class T>
    using range_reference_type = decltype(*std::declval<iterator_type<T>&>());
    template <class T>
    using range_rvalue_reference_type = decltype(std::move(*std::declval<iterator_type<T>&>()));

    template <class T, class... Args>
    struct is_curly_bracket_constructible : decltype(detail::is_curly_bracket_constructible_impl<T, Args...>(0)) {};
    template <class T, class... Args>
    inline constexpr bool is_curly_bracket_constructible_v = is_curly_bracket_constructible<T, Args...>::value;
}
