#include "kl/ctti.hpp"
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

namespace inner {
class T : public ::A, public S
{
public:
    explicit T(std::string d, std::vector<std::string> e)
        : d(std::move(d)), e(std::move(e))
    {
    }

private:
    friend struct kl::type_info<T>; // because `d` and `e` are private
    std::string d;
    std::vector<std::string> e;
};
} // namespace inner
} // namespace ns

struct B : A
{
    int zzz;
};

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

KL_DEFINE_REFLECTABLE(std, string, _)
KL_DEFINE_REFLECTABLE(A, (x, y))
KL_DEFINE_REFLECTABLE(ns, S, (a, b, c, aa))
KL_DEFINE_REFLECTABLE_DERIVED(ns::inner, T, (A, ns::S), (d, e))
KL_DEFINE_REFLECTABLE_DERIVED(B, A, (zzz))

namespace {

std::ostream& operator<<(std::ostream& os, const A& a)
{
    kl::ctti::reflect(a, [&os](auto fi) {
        os << fi.name() << ": " << fi.get() << "\n";
    });
    return os;
}
} // namespace anonymous

TEST_CASE("ctti")
{
    using namespace std::string_literals;

    SECTION("type not registered")
    {
        using T = std::vector<int>;
        REQUIRE(!kl::ctti::is_reflectable<T>());
        REQUIRE(kl::ctti::num_fields<T>() == 0);
        REQUIRE(kl::ctti::total_num_fields<T>() == 0);
        REQUIRE(kl::ctti::full_name<T>() == typeid(T).name());
        REQUIRE(kl::ctti::name<T>() == typeid(T).name());

        static_assert(
            std::is_same<kl::ctti::base_types<T>, kl::type_pack<>>::value,
            "???");
    }

    SECTION("std::string")
    {
        REQUIRE(kl::ctti::is_reflectable<std::string>());
        REQUIRE(kl::ctti::num_fields<std::string>() == 0);
        REQUIRE(kl::ctti::total_num_fields<std::string>() == 0);
        REQUIRE(kl::ctti::full_name<std::string>() == "std::string"s);
        REQUIRE(kl::ctti::name<std::string>() == "string"s);

        std::string v = "test";
        kl::ctti::reflect(v, [](auto fi) {
            REQUIRE(false);
        });
    }

    SECTION("global type A")
    {
        REQUIRE(kl::ctti::is_reflectable<A>());
        REQUIRE(kl::ctti::num_fields<A>() == 2);
        REQUIRE(kl::ctti::total_num_fields<A>() == 2);
        REQUIRE(kl::ctti::full_name<A>() == "A"s);
        REQUIRE(kl::ctti::name<A>() == "A"s);

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

    SECTION("global type B, derives from A")
    {
        REQUIRE(kl::ctti::is_reflectable<B>());
        REQUIRE(kl::ctti::num_fields<B>() == 1);
        REQUIRE(kl::ctti::total_num_fields<B>() == 3);
        REQUIRE(kl::ctti::full_name<B>() == "B"s);
        REQUIRE(kl::ctti::name<B>() == "B"s);

        static_assert(
            std::is_same<kl::ctti::base_types<B>, kl::type_pack<A>>::value,
            "???");

        std::ostringstream ss;
        B b;
        b.x = "QWE";
        b.y = {0, 1337};
        b.zzz = 123;
        b.z = 0;
        kl::ctti::reflect(b, [&ss](auto fi) {
            ss << fi.name() << ": " << fi.get() << "\n";
        });

        REQUIRE(ss.str() == "x: QWE\ny: 0, 1337\nzzz: 123\n");

        static_assert(std::is_same<kl::ctti::type<B, 0>, int>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<B, 0>, int&>::value, "???");
        static_assert(std::is_same<kl::ctti::at_type<const B, 0>, const int&>::value, "???");

        const B cb;

        static_assert(std::is_same<decltype(kl::ctti::at<0>(b)), int&>::value, "???");
        static_assert(std::is_same<decltype(kl::ctti::at<0>(cb)), const int&>::value, "???");

        REQUIRE(kl::ctti::at<0>(b) == 123);

        using namespace std::string_literals;

        REQUIRE((kl::ctti::field_name<B, 0>()) == "zzz"s);
    }

    SECTION("type S in namespace ns with std::array<>")
    {
        REQUIRE(kl::ctti::is_reflectable<ns::S>());
        REQUIRE(kl::ctti::num_fields<ns::S>() == 4);
        REQUIRE(kl::ctti::total_num_fields<ns::S>() == 4);
        REQUIRE(kl::ctti::full_name<ns::S>() == "ns::S"s);
        REQUIRE(kl::ctti::name<ns::S>() == "S"s);

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

    SECTION("type T with multi-inheritance in 2-level deep namespace")
    {
        using T = ns::inner::T;

        REQUIRE(kl::ctti::is_reflectable<T>());
        REQUIRE(kl::ctti::num_fields<T>() == 2);
        REQUIRE(kl::ctti::total_num_fields<T>() == 2 + 2+ 4);
        REQUIRE(kl::ctti::full_name<T>() == "ns::inner::T"s);
        REQUIRE(kl::ctti::name<T>() == "T"s);

        T t{"HELLO",{"WORLD", "Hello"}};
        t.a = 2;
        t.b = true;
        t.c = {{2.71f, 3.14f, 1.67f}};

        std::ostringstream ss;
        ss << std::boolalpha;
        kl::ctti::reflect(t, [&ss](auto fi) {
            ss << fi.name() << ": " << fi.get() << "\n";
        });
        REQUIRE(ss.str() == "x: \ny: .\na: 2\nb: true\nc: 2.71, 3.14, "
                            "1.67\naa: x: \ny: .\n\nd: HELLO\ne: WORLD, "
                            "Hello\n");
    }
}
