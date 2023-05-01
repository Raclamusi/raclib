#pragma once

#include <atcoder/all>
#include "scan.hpp"
#include "print.hpp"

namespace raclib {
    template <int Mod, class CharT, class Traits>
    struct scanner<atcoder::static_modint<Mod>, CharT, Traits> {
        using result_type = atcoder::static_modint<Mod>;

        result_type scan(std::basic_istream<CharT, Traits>& is) {
            long long t;
            is >> t;
            return result_type{ t };
        }
    };

    template <int ID, class CharT, class Traits>
    struct scanner<atcoder::dynamic_modint<ID>, CharT, Traits> {
        using result_type = atcoder::dynamic_modint<ID>;

        result_type scan(std::basic_istream<CharT, Traits>& is) {
            long long t;
            is >> t;
            return result_type{ t };
        }
    };

    template <int Mod, class CharT, class Traits>
    struct printer<atcoder::static_modint<Mod>, CharT, Traits> {
        void print(std::basic_ostream<CharT, Traits>& os, print_alignment align, const atcoder::static_modint<Mod>& x) {
            os << x.val();
        }
    };

    template <int ID, class CharT, class Traits>
    struct printer<atcoder::dynamic_modint<ID>, CharT, Traits> {
        void print(std::basic_ostream<CharT, Traits>& os, print_alignment align, const atcoder::dynamic_modint<ID>& x) {
            os << x.val();
        }
    };
}
