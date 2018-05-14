#include <iostream>
#include <tuple>

struct EnumInfo
{
    const char* fullname{};
    const char* shortname{};
};

namespace compile_time
{
    // compile-time integral values
    template<auto u>
    using cint = typename std::integral_constant<decltype(u), u>;

    #define CINT(x) cint<x>{}

    // compile-time strings
    template<char ...s>
    constexpr char const string_storage[sizeof...(s) + 1] = {s..., '\0'};

    template<char ...s>
    struct cstring { static constexpr const char* c_str() { return &string_storage<s...>[0]; } };

    template <typename S, std::size_t ...N>
    constexpr cstring<S::get()[N]...> prepare_impl(S, std::index_sequence<N...>) { return {}; }

    template <typename S>
    constexpr decltype(auto) prepare(S s) { return prepare_impl(s, std::make_index_sequence<sizeof(S::get()) - 1>{}); }

    #define CSTR(s) \
        compile_time::prepare([]{ \
            struct tmp { static constexpr decltype(auto) get() { return s; } }; \
            return tmp{}; \
            }() )

} // end of compile-time namespace

using namespace compile_time;

#define ENUMINFO(x, y, z) CINT(x), CSTR(y), CSTR(z)

enum class ColorTypeDef { Red, Green, Blue, Orange, DEFAULT = Red };

template<typename ENUMTYPE, typename... Ts>
struct ColorType
{
    using type = ENUMTYPE;
    using TUPLE = std::tuple<Ts...>;

    ENUMTYPE value;

    constexpr ColorType(const Ts...) : value{ENUMTYPE::DEFAULT}
    {
    }

    constexpr ColorType(ENUMTYPE value, const Ts...) : value{value}
    {
    }

    template<ENUMTYPE ENUMVAL, size_t i = 0>
    static constexpr auto infoOf() -> EnumInfo
    {
        const auto sz = std::tuple_size_v<TUPLE>;
        if constexpr(std::is_same_v<cint<ENUMVAL>, std::tuple_element_t<i, TUPLE>>)
            return { std::tuple_element_t<i+1, TUPLE>::c_str(), std::tuple_element_t<i+2, TUPLE>::c_str() };
        else if constexpr(i < sz-1)
            return infoOf<ENUMVAL, i+1>();
        else
            return {"", ""};
    }
};

template<typename FIRST, typename ...REST>
ColorType(FIRST, const REST...) -> ColorType<typename FIRST::value_type, FIRST, REST...>;

int main()
{
    using namespace std;

    ColorType ct{
//        ColorTypeDef::Red,
        ENUMINFO(ColorTypeDef::Red, "Red", "R"),
        ENUMINFO(ColorTypeDef::Green, "Green", "G"),
        ENUMINFO(ColorTypeDef::Blue, "Blue", "B")
    };

    cout << ct.infoOf<ColorTypeDef::Green>().fullname << endl;
//    cout << ct.infoOf<cint<ColorTypeDef::Green>>().shortname << endl;
    return string(ct.infoOf<ColorTypeDef::Green>().shortname).size();
//    return 0;
}