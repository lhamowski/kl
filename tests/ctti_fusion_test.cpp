#include "kl/ctti_fusion.hpp"
#include "kl/stream_join.hpp"

#include <catch/catch.hpp>

#include <string>
#include <vector>
#include <array>
#include <sstream>

namespace {

struct A
{
    std::string x;
    std::vector<int> y;
    int z; // Not reflectable
};

namespace ns {
struct S
{
    int a;
    bool b;
    std::array<float, 3> c;
    A aa;
};
} // namespace ns

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    return os << kl::stream_join(v);
}

template <typename T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& v)
{
    return os << kl::stream_join(v);
}
} // namespace anonymous

BOOST_FUSION_ADAPT_STRUCT(A, x, y)
BOOST_FUSION_ADAPT_STRUCT(ns::S, a, b, c, aa)

namespace {

std::ostream& operator<<(std::ostream& os, const A& a)
{
    kl::ctti::reflect(a, [&os](auto fi) {
        os << fi.name() << ": " << fi.get() << "\n";
    });
    return os;
}
} // namespace anonymous

TEST_CASE("ctti_fusion")
{
    SECTION("global type A")
    {
        REQUIRE(kl::ctti::is_reflectable<A>());
        REQUIRE(kl::ctti::num_fields<A>() == 2);
        REQUIRE(kl::ctti::total_num_fields<A>() == 2);
        REQUIRE(kl::ctti::full_name<A>() == typeid(A).name());
        REQUIRE(kl::ctti::name<A>() == typeid(A).name());

        static_assert(std::is_same<kl::ctti::type<A>, A>::value, "???");
        static_assert(
            std::is_same<kl::ctti::base_types<A>, kl::type_pack<>>::value,
            "???");

        std::ostringstream ss;
        A a = {"ZXC", {1,2,3}, 0};
        kl::ctti::reflect(a, [&ss](auto fi){
            ss << fi.name() << ": " << fi.get() << "\n";
        });

        REQUIRE(ss.str() == "x: ZXC\ny: 1, 2, 3\n");
    }

    SECTION("type S in namespace ns with std::array<>")
    {
        REQUIRE(kl::ctti::is_reflectable<ns::S>());
        REQUIRE(kl::ctti::num_fields<ns::S>() == 4);
        REQUIRE(kl::ctti::total_num_fields<ns::S>() == 4);
        REQUIRE(kl::ctti::full_name<ns::S>() == typeid(ns::S).name());
        REQUIRE(kl::ctti::name<ns::S>() == typeid(ns::S).name());

        const ns::S s = {5, false, {3.14f}, A{"ZXC", {1, 2, 3, 4, 5, 6}, 0}};

        std::ostringstream ss;
        ss << std::boolalpha;
        kl::ctti::reflect(s, [&ss](auto fi) {
            ss << fi.name() << ": " << fi.get() << "\n";
        });

        REQUIRE(ss.str() == "a: 5\nb: false\nc: 3.14, 0, 0\naa: x: ZXC\ny: 1, "
                            "2, 3, 4, 5, 6\n\n");
    }
}
