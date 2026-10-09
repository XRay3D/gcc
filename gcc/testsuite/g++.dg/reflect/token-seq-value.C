// std::meta::token_value_of (an extension to P3294, as in EDG and
// clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

constexpr int operator ""_km (unsigned long long v) { return int (v); }

static_assert ([: token_value_of (^^{ 42 }) :] == 42);
static_assert (__is_same (decltype ([: token_value_of (^^{ 42 }) :]), int));
static_assert ([: token_value_of (^^{ 0x10 }) :] == 16);
static_assert ([: token_value_of (^^{ 42u }) :] == 42u);
static_assert (__is_same (decltype ([: token_value_of (^^{ 42u }) :]), unsigned));
static_assert ([: token_value_of (^^{ 2.5 }) :] == 2.5);
static_assert (__is_same (decltype ([: token_value_of (^^{ 2.5 }) :]), double));
static_assert ([: token_value_of (^^{ 2.5f }) :] == 2.5f);
static_assert ([: token_value_of (^^{ 'a' }) :] == 'a');
static_assert (__is_same (decltype ([: token_value_of (^^{ 'a' }) :]), char));
static_assert ([: token_value_of (^^{ L'a' }) :] == L'a');
static_assert ([: token_value_of (^^{ u8'a' }) :] == u8'a');
static_assert ([: token_value_of (^^{ true }) :] == true);
static_assert ([: token_value_of (^^{ false }) :] == false);
static_assert (__is_same (decltype ([: token_value_of (^^{ true }) :]), bool));
static_assert (token_value_of (^^{ 'a' }) == reflect_constant ('a'));
static_assert (token_value_of (^^{ 42 }) == reflect_constant (42));

// A string literal yields the string literal object.
static_assert (sizeof ([: token_value_of (^^{ "str" }) :]) == 4);
static_assert ([: token_value_of (^^{ "str" }) :][0] == 's');
static_assert ([: token_value_of (^^{ "str" }) :][3] == '\0');
// Its type is that of the object std::meta::reflect_constant_string gives.
static_assert (__is_same (decltype ([: token_value_of (^^{ "str" }) :]),
			  decltype ([: reflect_constant_string ("str") :])));
static_assert (token_value_of (^^{ "str" }) == reflect_constant_string ("str"));
static_assert ([: token_value_of (^^{ L"w" }) :][0] == L'w');
static_assert ([: token_value_of (^^{ u8"é" }) :][1] == u8"é"[1]);
static_assert ([: token_value_of (^^{ "a\tb" }) :][1] == '\t');
static_assert ([: token_value_of (^^{ R"(a\tb)" }) :][1] == '\\');

// The value is that of the first token.
static_assert ([: token_value_of (^^{ 7 + 8 }) :] == 7);
static_assert ([: token_value_of (tokens_of (^^{ 7 + 8 })[2]) :] == 8);

// User-defined literals: the value of the literal without its suffix.
static_assert ([: token_value_of (^^{ 42_km }) :] == 42);
static_assert ([: token_value_of (^^{ 2.5_km }) :] == 2.5);
static_assert ([: token_value_of (^^{ 'c'_x }) :] == 'c');
static_assert ([: token_value_of (^^{ "s"_x }) :][0] == 's');

// Interpolated values.
static_assert ([: token_value_of (^^{ \val(42) }) :] == 42);
static_assert (__is_same (decltype ([: token_value_of (^^{ \val(42) }) :]), int));
static_assert (token_value_of (^^{ \val(42) }) == reflect_constant (42));
struct P { int a, b; };
static_assert ([: token_value_of (^^{ \val(P{1, 2}) }) :].b == 2);

// An interpolated reflection yields the reflection itself.
constexpr info r = ^^int;
static_assert (token_value_of (tokens_of (^^{ \[: r :] })[1]) == ^^int);
static_assert (token_value_of (tokens_of (^^{ \[: ^^P :] })[1]) == ^^P);
constexpr int limit = 42;
constexpr info val_limit = tokens_of (^^{ \val(^^limit) })[0];
static_assert (token_kind_of (val_limit) == token_kind::interpolated);
static_assert (token_value_of (val_limit) == ^^limit);
static_assert ([: token_value_of (val_limit) :] == 42);

// An identifier has no value; the entity it names is found by injecting it.
consteval {
  queue_injection (^^{ constexpr info found_limit = ^^\{tokens_of (^^{ limit })[0]}; });
}
static_assert (found_limit == ^^limit);

constexpr info e1 = token_value_of (^^{ x });  // { dg-error "'the token is an identifier, which has no value'" }
constexpr info e2 = token_value_of (^^{ int });  // { dg-error "'the token is a keyword, which has no value'" }
constexpr info e3 = token_value_of (^^{ + });  // { dg-error "'the token is a punctuator, which has no value'" }
constexpr info e4 = token_value_of (^^{ nullptr });  // { dg-error "'the token is a keyword, which has no value'" }
constexpr info e5 = token_value_of (^^{});  // { dg-error "'token sequence is empty'" }
constexpr info e6 = token_value_of (^^int);  // { dg-error "'reflection does not represent a token sequence'" }
