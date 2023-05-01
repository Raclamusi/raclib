#pragma once

#include <iostream>
#include <iomanip>
#include <tuple>
#include <utility>
#include <iterator>
#include <type_traits>
#include "traits.hpp"

namespace raclib {
    namespace detail {
        template <class T, class CharT, class Traits, class = void>
        struct is_default_printable : std::false_type {};
        template <class T, class CharT, class Traits>
        struct is_default_printable<T, CharT, Traits, decltype(std::declval<std::basic_ostream<CharT, Traits>&>() << std::declval<const T&>(), void())> : std::true_type {};
        template <class T, class CharT, class Traits>
        inline constexpr bool is_default_printable_v = is_default_printable<T, CharT, Traits>::value;

        template <class Printer, class = void>
        struct is_multi_output : std::false_type {};
        template <class Printer>
        struct is_multi_output<Printer, typename Printer::is_multi_output> : std::true_type {};
        template <class Printer>
        inline constexpr bool is_multi_output_v = is_multi_output<Printer>::value;
    }

    enum class print_alignment {
        automatic,
        horizontal,
        vertical,
    };

    template <class T, class CharT = char, class Traits = std::char_traits<CharT>, class = void>
    struct printer {
        void print(std::basic_ostream<CharT, Traits>& os, print_alignment, const T& x) {
            os << x;
        }
    };

    template <class T, class CharT, class Traits>
    struct printer<T, CharT, Traits, std::enable_if_t<std::is_floating_point_v<T>>> {
        void print(std::basic_ostream<CharT, Traits>& os, print_alignment, const T& x) {
            os << std::fixed << std::setprecision(10) << x;
        }
    };

    template <class T, class CharT, class Traits>
    struct printer<T, CharT, Traits, std::enable_if_t<not detail::is_default_printable_v<T, CharT, Traits> && is_tuple_like_v<T>>> {
        using is_multi_output = void;

        void print(std::basic_ostream<CharT, Traits>& os, print_alignment align, const T& x) {
            print_impl(os, align, x, std::make_index_sequence<std::tuple_size_v<T>>{});
        }

    private:
        template <std::size_t I>
        using element_printer_t = printer<std::decay_t<std::tuple_element_t<I, T>>, CharT, Traits>;

        template <std::size_t... I>
        void print_impl(std::basic_ostream<CharT, Traits>& os, print_alignment align, const T& x, std::index_sequence<I...>) {
            using std::get;
            constexpr char auto_del = (detail::is_multi_output_v<element_printer_t<I>> || ...) ? '\n' : ' ';
            char del
                = align == print_alignment::horizontal ? ' '
                : align == print_alignment::vertical ? '\n'
                : auto_del;
            (element_printer_t<I>{}.print((I == 0 ? os : os << del), align, get<I>(x)), ...);
        }
    };

    template <class T, class CharT, class Traits>
    struct printer<T, CharT, Traits, std::enable_if_t<not detail::is_default_printable_v<T, CharT, Traits> && not is_tuple_like_v<T> && is_range_v<T>>> {
        using is_multi_output = void;

        void print(std::basic_ostream<CharT, Traits>& os, print_alignment align, const T& x) {
            using std::begin, std::end;
            using element_printer_t = printer<std::decay_t<range_value_type<T>>, CharT, Traits>;
            constexpr char auto_del = detail::is_multi_output_v<element_printer_t> ? '\n' : ' ';
            char del
                = align == print_alignment::horizontal ? ' '
                : align == print_alignment::vertical ? '\n'
                : auto_del;
            element_printer_t printer;
            auto first = begin(x);
            auto last = end(x);
            if (first != last) {
                printer.print(os, align, *first);
                ++first;
            }
            while (first != last) {
                os << del;
                printer.print(os, align, *first);
                ++first;
            }
        }
    };

    template <class CharT, class Traits, class... Args>
    void print(std::basic_ostream<CharT, Traits>& os, print_alignment align, Args&&... args) {
        printer<std::tuple<Args...>, CharT, Traits>{}.print(os, align, { std::forward<Args>(args)... });
    }

    template <class CharT, class Traits, class... Args>
    void print(std::basic_ostream<CharT, Traits>& os, Args&&... args) {
        print(os, print_alignment::automatic, std::forward<Args>(args)...);
    }

    template <class... Args>
    void print(print_alignment align, Args&&... args) {
        print(std::cout, align, std::forward<Args>(args)...);
    }

    template <class... Args>
    void print(Args&&... args) {
        print(std::cout, print_alignment::automatic, std::forward<Args>(args)...);
    }

    template <class CharT, class Traits, class... Args>
    void println(std::basic_ostream<CharT, Traits>& os, print_alignment align, Args&&... args) {
        print(os, align, std::forward<Args>(args)...);
        os << std::endl;
    }

    template <class CharT, class Traits, class... Args>
    void println(std::basic_ostream<CharT, Traits>& os, Args&&... args) {
        println(os, print_alignment::automatic, std::forward<Args>(args)...);
    }

    template <class... Args>
    void println(print_alignment align, Args&&... args) {
        println(std::cout, align, std::forward<Args>(args)...);
    }

    template <class... Args>
    void println(Args&&... args) {
        println(std::cout, print_alignment::automatic, std::forward<Args>(args)...);
    }
}
