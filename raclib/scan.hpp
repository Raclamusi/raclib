#pragma once

#include <iostream>
#include <tuple>
#include <utility>
#include <iterator>
#include <vector>
#include <array>
#include <type_traits>
#include "traits.hpp"

namespace raclib {
    namespace detail {
        template <class T, class CharT, class Traits, class = void>
        struct is_default_scannable : std::false_type {};
        template <class T, class CharT, class Traits>
        struct is_default_scannable<T, CharT, Traits, decltype(std::declval<std::basic_istream<CharT, Traits>&>() >> std::declval<T&>(), void())> : std::true_type {};
        template <class T, class CharT, class Traits>
        inline constexpr bool is_default_scannable_v = is_default_scannable<T, CharT, Traits>::value;

        template <class C, class = void>
        struct is_reservable : std::false_type {};
        template <class C>
        struct is_reservable<C, decltype(std::declval<C&>().reserve(std::declval<range_size_type<C>>()), void())> : std::true_type {};
        template <class C>
        inline constexpr bool is_reservable_v = is_reservable<C>::value;

        template <class C, class = void>
        struct is_back_insertable : std::false_type {};
        template <class C>
        struct is_back_insertable<C, decltype(std::declval<C&>().push_back(std::declval<range_value_type<C>>()), void())> : std::true_type {};
        template <class C>
        inline constexpr bool is_back_insertable_v = is_back_insertable<C>::value;

        template <class C, class = void>
        struct is_insertable : std::false_type {};
        template <class C>
        struct is_insertable<C, decltype(std::declval<C&>().insert(std::declval<C&>().end(), std::declval<range_value_type<C>>()), void())> : std::true_type {};
        template <class C>
        inline constexpr bool is_insertable_v = is_insertable<C>::value;

        template <class C>
        struct is_insertable_container : std::conjunction<is_range<C>, std::disjunction<is_back_insertable<C>, is_insertable<C>>> {};
        template <class C>
        inline constexpr bool is_insertable_container_v = is_insertable_container<C>::value;

        template <class C>
        constexpr auto container_inserter(C& c) {
            if constexpr (is_back_insertable_v<C>) {
                return std::back_inserter(c);
            }
            else {
                return std::inserter(c, c.end());
            }
        }
    }

    namespace marker {
        struct decrement_t {} inline constexpr decrement;
    }

    template <class T = void, class CharT = char, class Traits = std::char_traits<CharT>, class = void>
    struct scanner {
        using result_type = T;

        T scan(std::basic_istream<CharT, Traits>& is) {
            T t;
            is >> t;
            return t;
        }

        T scan(std::basic_istream<CharT, Traits>& is, marker::decrement_t) {
            T t = scan(is);
            --t;
            return t;
        }
    };

    template <class CharT, class Traits, class... Args>
    class lazy_scanner {
    private:
        std::basic_istream<CharT, Traits>& is;
        std::tuple<Args...> args_tuple;

    public:
        lazy_scanner(std::basic_istream<CharT, Traits>& is, Args&&... args)
            : is{ is }, args_tuple{ std::forward<Args>(args)... } {}
        lazy_scanner(const lazy_scanner&) = delete;
        lazy_scanner(lazy_scanner&&) = delete;

        template <class T>
        T scan() && {
            return std::apply([&](Args&&... args) {
                return scanner<T, CharT, Traits>{}.scan(is, std::forward<Args>(args)...);
            }, std::move(args_tuple));
        }

        template <class T>
        operator T() && {
            return std::move(*this).template scan<T>();
        }
    };

    template <class CharT, class Traits>
    struct scanner<void, CharT, Traits> {
        using result_type = void;

        template <class... Args>
        lazy_scanner<CharT, Traits, Args...> scan(std::basic_istream<CharT, Traits>& is, Args&&... args) {
            return { is, std::forward<Args>(args)... };
        }
    };

    template <class T, class CharT, class Traits>
    struct scanner<T, CharT, Traits, std::enable_if_t<not detail::is_default_scannable_v<T, CharT, Traits> && is_tuple_like_v<T>>> {
        using result_type = T;

        template <class... Args>
        T scan(std::basic_istream<CharT, Traits>& is, Args&&... args) {
            return scan_impl(is, std::make_index_sequence<std::tuple_size_v<T>>{}, std::forward<Args>(args)...);
        }

    private:
        template <std::size_t I>
        using element_scanner_t = scanner<std::tuple_element_t<I, T>, CharT, Traits>;

        template <std::size_t... I, class... Args>
        T scan_impl(std::basic_istream<CharT, Traits>& is, std::index_sequence<I...>, Args&&... args) {
            if constexpr (is_curly_bracket_constructible_v<T, std::tuple_element_t<I, T>...>) {
                return T{ element_scanner_t<I>{}.scan(is, args...)... };
            }
            else if constexpr (std::is_constructible_v<T, std::tuple_element_t<I, T>...>) {
                return T(element_scanner_t<I>{}.scan(is, args...)...);
            }
            else {
                using std::get;
                T t;
                ((get<I>(t) = element_scanner_t<I>{}.scan(is, args...)), ...);
                return t;
            }
        }
    };

    template <class T, class CharT, class Traits>
    struct scanner<T, CharT, Traits, std::enable_if_t<not detail::is_default_scannable_v<T, CharT, Traits> && not is_tuple_like_v<T> && detail::is_insertable_container_v<T>>> {
        using result_type = T;

        template <class... Args>
        T scan(std::basic_istream<CharT, Traits>& is, range_size_type<T> n, Args&&... args) {
            T t;
            if constexpr (detail::is_reservable_v<T>) {
                t.reserve(n);
            }
            auto inserter = detail::container_inserter(t);
            scanner<range_value_type<T>, CharT, Traits> scanner;
            for (range_size_type<T> i{}; i < n; ++i) {
                *inserter++ = scanner.scan(is, args...);
            }
            return t;
        }
    };

    template <class T, std::size_t N, class CharT, class Traits>
    struct scanner<T[N], CharT, Traits> : scanner<std::array<typename scanner<T>::result_type, N>, CharT, Traits> {};

    template <class T, class CharT, class Traits>
    struct scanner<T[], CharT, Traits> : scanner<std::vector<typename scanner<T>::result_type>, CharT, Traits> {};

    template <class T = void, class CharT, class Traits, class... Args>
    auto scan(std::basic_istream<CharT, Traits>& is, Args&&... args) {
        return scanner<T, CharT, Traits>{}.scan(is, std::forward<Args>(args)...);
    }

    template <class T = void, class... Args>
    auto scan(Args&&... args) {
        return scan<T>(std::cin, std::forward<Args>(args)...);
    }
}
