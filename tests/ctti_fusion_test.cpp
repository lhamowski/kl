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

        static_assert(
            std::is_same<kl::ctti::base_types<A>, kl::type_pack<>>::value,
            "???");

        std::ostringstream ss;
        A a = {"ZXC", {1,2,3}, 0};
        kl::ctti::reflect(a, [&ss](auto fi){
            ss << fi.name() << ": " << fi.get() << "\n";
        });

        REQUIRE(ss.str() == "x: ZXC\ny: 1, 2, 3\n");

        static_assert(std::is_same<kl::ctti::type<A, 0>, std::string>::value, "???");
        static_assert(std::is_same<kl::ctti::type<A, 1>, std::vector<int>>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<A, 0>, std::string&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<A, 1>, std::vector<int>&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const A, 0>, const std::string&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const A, 1>, const std::vector<int>&>::value, "???");

        const A ca = {"ZXC",{1,2,3}, 0};

        static_assert(std::is_same<decltype(kl::ctti::at<0>(a)), std::string&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<0>(ca)), const std::string&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<1>(a)), std::vector<int>&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<1>(ca)), const std::vector<int>&>::value, "???");

        REQUIRE(kl::ctti::at<0>(ca) == "ZXC");
        REQUIRE(kl::ctti::at<1>(ca) == (std::vector<int>{1, 2, 3}));

        using namespace std::string_literals;

        REQUIRE((kl::ctti::field_name<A, 0>()) == "x"s);
        REQUIRE((kl::ctti::field_name<A, 1>()) == "y"s);
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

        static_assert(std::is_same<kl::ctti::type<ns::S, 0>, int>::value, "???");
        static_assert(std::is_same<kl::ctti::type<ns::S, 1>, bool>::value, "???");
        static_assert(std::is_same<kl::ctti::type<ns::S, 2>, std::array<float, 3>>::value, "???");
        static_assert(std::is_same<kl::ctti::type<ns::S, 3>, A>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<ns::S, 0>, int&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<ns::S, 1>, bool&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<ns::S, 2>, std::array<float, 3>&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<ns::S, 3>, A&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const ns::S, 0>, const int&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const ns::S, 1>, const bool&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const ns::S, 2>, const std::array<float, 3>&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const ns::S, 3>, const A&>::value, "???");

        ns::S ms = {5, false,{3.14f}, A{"ZXC",{1, 2, 3, 4, 5, 6}, 0}};

        static_assert(std::is_same<decltype(kl::ctti::at<0>(ms)), int&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<0>(s)), const int&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<1>(ms)), bool&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<1>(s)), const bool&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<2>(ms)), std::array<float, 3>&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<2>(s)), const std::array<float, 3>&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<3>(ms)), A&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<3>(s)), const A&>::value, "???");

        REQUIRE(kl::ctti::at<0>(ms) == 5);
        REQUIRE(kl::ctti::at<1>(ms) == false);
        REQUIRE(kl::ctti::at<2>(ms) == (std::array<float, 3>{3.14f, 0.0f, 0.0f}));

        using namespace std::string_literals;

        REQUIRE((kl::ctti::field_name<ns::S, 0>()) == "a"s);
        REQUIRE((kl::ctti::field_name<ns::S, 1>()) == "b"s);
        REQUIRE((kl::ctti::field_name<ns::S, 2>()) == "c"s);
        REQUIRE((kl::ctti::field_name<ns::S, 3>()) == "aa"s);
    }
}
