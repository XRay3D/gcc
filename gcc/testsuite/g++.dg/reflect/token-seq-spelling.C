// std::meta::token_spelling_of and display_string_of of a token sequence
// (an extension to P3294, as in EDG and clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

static_assert (token_spelling_of (^^{ int x = 0x10 + 'a'; }) == "int x = 0x10 + 'a' ;");
static_assert (token_spelling_of (^^{}) == "");
static_assert (token_spelling_of (^^{ x }) == "x");
static_assert (token_spelling_of (^^{ a bitor b and c not_eq d }) == "a bitor b and c not_eq d");
static_assert (token_spelling_of (^^{ f("s", 2.5, true); }) == "f ( \"s\" , 2.5 , true ) ;");
static_assert (token_spelling_of (^^{ a :: b -> c }) == "a :: b -> c");
static_assert (token_spelling_of (^^{ u8"x" R"(raw)" L'w' 1'000 0x1p3 1e3 }) == "u8\"x\" R\"(raw)\" L'w' 1'000 0x1p3 1e3");
static_assert (token_spelling_of (^^{ 42_km 2.5_km 'c'_x "s"_x }) == "42_km 2.5_km 'c'_x \"s\"_x");
static_assert (token_spelling_of (^^{ [: :] ^^ ... <=> }) == "[: :] ^^ ... <=>");

// Line breaks and macro expansions do not show.
#define TWICE(x) x x
static_assert (token_spelling_of (^^{ TWICE(a)
				       b }) == "a a b");
#define NUMS(a, b) a b
static_assert (token_spelling_of (^^{ NUMS(0x1, 2u) }) == "0x1 2u");

// Interpolated tokens.
constexpr const char *name = "a";
static_assert (token_spelling_of (^^{ \val(42) \str("hi") \[name, 1] }) == "42 \"hi\" a1");
static_assert (token_spelling_of (^^{ \[: ^^int :] }) == "[: ^^int :]");
static_assert (token_spelling_of (tokens_of (^^{ x + y })[1]) == "+");

// UTF-8 variant.
static_assert (u8token_spelling_of (^^{ int x ; }) == u8"int x ;");

// display_string_of gives the same text.
constexpr info seq = ^^{ int x = 0x10 + 'a';
  if (x > 3) f("s", 2.5, true); };
consteval bool display_agrees (info s)
{
  for (info t : tokens_of (s))
    if (display_string_of (t) != token_spelling_of (t))
      return false;
  return display_string_of (s) == token_spelling_of (s);
}
static_assert (display_agrees (seq));
static_assert (u8display_string_of (seq) == u8token_spelling_of (seq));

// Rebuild, replacing each identifier "x" by "y", comparing spellings.
consteval info rename_x (info s)
{
  info out = ^^{};
  for (info t : tokens_of (s))
    if (token_kind_of (t) == token_kind::identifier
	&& token_spelling_of (t) == "x")
      out = ^^{ \{out} y };
    else
      out = ^^{ \{out} \{t} };
  return out;
}
static_assert (token_spelling_of (rename_x (^^{ x + x * z })) == "y + y * z");

constexpr auto s1 = token_spelling_of (^^int);  // { dg-error "uncaught exception of type 'std::meta::exception'; 'what\\(\\)': 'reflection does not represent a token sequence'" }
