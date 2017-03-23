#include "catch.hpp"

#include <type_traits>
#include <tuple>
#include <array>


//==============================================================================
// Applying function on std::tuple

namespace
{
    struct my_func
    {
        int val_ = 0;

        void operator () (int i)
        { val_ += i; }

        void operator () (double d)
        { val_ += 10; }

        template <typename T>
        void operator () (T)
        { }
    };


    template <typename F, typename... T, std::size_t... i>
    auto my_foreach_impl(F && f, std::tuple<T...> const& t, std::index_sequence<i...>)
    {
        (void) std::initializer_list<int> { (f(std::get<i>(t)), 0)... };
        return f;
    }

    template <typename F, typename... T>
    auto my_foreach(F && f, std::tuple<T...> const& t)
    {
        return my_foreach_impl(
                    std::forward<F>(f),
                    t,
                    std::index_sequence_for<T...>{}
                );
    }
}   // un-named namespace


TEST_CASE("foreach on std::tuple element", "[tmp]")
{
    auto t = std::make_tuple(10, "abc", 100.0);

    static_assert(
        std::is_same<
                std::tuple<int, char const *, double>,
                decltype(t)
        >()
    );

    auto result = my_foreach(my_func{}, t);
    REQUIRE(result.val_ == 20);
}

#if __clang_major__ >= 4    // if C++17 std::apply is available,
struct my_func_for_apply
{
    auto operator () (int i, char const *, double d)
    {
        return i + 20;
    }

    auto operator () (int i, char const *)
    {
        return i + 20;
    }

    template <typename... T>
    auto operator () (T... t)
    {
        return (t + ...);   // C++17 fold expression. fold-right.
                            // return the sum.
    }
};

// std::apply works on any tuple-like objects
//  that supports std::get and std::tuple_size.
//  ex.> std::tuple, std::pair, std::array and etc.
TEST_CASE("std::apply with std::tuple", "[tmp]")
{
    auto t = std::make_tuple(10, "abc", 100.0);

    static_assert(std::tuple_size_v<decltype(t)> == 3);
    REQUIRE(std::get<0>(t) == 10);

    REQUIRE(std::apply(my_func_for_apply{}, t) == 30);
}

TEST_CASE("std::apply with std::pair", "[tmp]")
{
    auto p = std::make_pair(10, "abc");

    static_assert(std::tuple_size_v<decltype(p)> == 2);
    REQUIRE(std::get<0>(p) == 10);

    REQUIRE(std::apply(my_func_for_apply{}, p) == 30);
}

TEST_CASE("std::apply with std::array", "[tmp]")
{
    std::array<std::size_t, 5> arr = { 1, 2, 3, 4, 5 };

    static_assert(std::tuple_size_v<decltype(arr)> == 5);
    REQUIRE(std::get<0>(arr) == 1);

    REQUIRE(std::apply(my_func_for_apply{}, arr) == 15);
}
#endif


//==============================================================================
// reversing std::tuple's element types

template <typename Tuple, std::size_t SizeOfTuple, typename IndexSeq>
struct reverse_tuple_impl;

template <typename... params, std::size_t SizeOfTuple, size_t... i>
struct reverse_tuple_impl<std::tuple<params...>, SizeOfTuple, std::index_sequence<i...>>
{
    using type = std::tuple<
                        std::tuple_element_t<
                                SizeOfTuple - i - 1,
                                std::tuple<params...>
                        >...
                 >;
};

template <typename Tuple>
struct reverse_tuple;

template <typename... params>
struct reverse_tuple<std::tuple<params...>>
            : reverse_tuple_impl<
                    std::tuple<params...>,
                    sizeof...(params),
                    std::index_sequence_for<params...>
              >
{ };

TEST_CASE("reversing std::tuple's element types", "[tmp]")
{
    using tuple_t = std::tuple<int, std::string, double>;
    using reversed_t = std::tuple<double, std::string, int>;
    static_assert(
        std::is_same<
                reversed_t,
                reverse_tuple<tuple_t>::type
        >()
    );
}
