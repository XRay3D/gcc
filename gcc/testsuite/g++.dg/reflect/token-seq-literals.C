// P3294 token sequences: ^^{ ... } literals and their equality.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

#ifndef __GXX_TOKEN_INJECTION__
#error "__GXX_TOKEN_INJECTION__ not defined"
#endif
static_assert (__has_feature (token_injection));

namespace basic_literals {
constexpr auto t1 = ^^{ a + b };
constexpr info t2 = ^^{};
constexpr auto t3 = ^^{ int f() { return { 1 }; } };

static_assert (__is_same (decltype (t1), const info));
static_assert (__is_same (decltype (^^{ }), info));

static_assert (is_token_sequence (t1));
static_assert (is_token_sequence (t2));
static_assert (is_token_sequence (t3));
static_assert (!is_token_sequence (^^int));
static_assert (!is_token_sequence (^^::));
static_assert (!is_token_sequence (info{}));

static_assert (!is_empty_token_sequence (t1));
static_assert (is_empty_token_sequence (t2));
static_assert (!is_empty_token_sequence (^^int));

// Only braces have to be balanced.
constexpr auto t4 = ^^{ ( [ < };
constexpr auto t5 = ^^{ ) ] > :] [: };
static_assert (is_token_sequence (t4) && is_token_sequence (t5));

// Arbitrary tokens: keywords, literals, punctuators.
constexpr auto t6 = ^^{ template <typename T> struct S { T t = 1.5f; }; };
constexpr auto t7 = ^^{ "string" 'c' 42 0x2a 1'000 u8"x" R"(raw)" nullptr };

// Nested braces.
constexpr auto t8 = ^^{ { { } } { } };
static_assert (!is_empty_token_sequence (t8));
}  // namespace basic_literals

namespace equality {
// Whitespace and comments are not significant.
constexpr auto t1 = ^^{ hello  = /* world */   "world" };
constexpr auto t2 = ^^{ hello="world" };
static_assert (t1 == t2);

static_assert (^^{ a + b } == ^^{a+b});
static_assert (^^{ a + b } != ^^{ a - b });
static_assert (^^{ a + b } != ^^{ a + c });
static_assert (^^{ a + b } != ^^{ a + b + c });
static_assert (^^{} == ^^{});
static_assert (^^{} != ^^{;});
static_assert (^^{ 1 } != ^^{ 1u });
static_assert (^^{ 1 } != ^^{ 0x1 });
static_assert (^^{ 1 } == ^^{ 1 });
static_assert (^^{ 'a' } != ^^{ 'b' });
static_assert (^^{ "a" } != ^^{ "b" });
static_assert (^^{ "a" } == ^^{ "a" });
static_assert (^^{ "a" } != ^^{ u8"a" });
static_assert (^^{ abc } != ^^{ ab c });
static_assert (^^{ int } != ^^int);
static_assert (^^{ int } != ^^{ long });
static_assert (^^{ >> } != ^^{ > > });
static_assert (^^{ bitor } != ^^{ | });
static_assert (^^{ [: } != ^^{ [ : });

// Concatenation preserves the identity of tokens.
constexpr auto abc = ^^{ abc };
constexpr auto def = ^^{ def };
constexpr auto both = ^^{ \{abc} \{def} };
static_assert (both == ^^{ abc def });
static_assert (both != ^^{ abcdef });

// Macros are expanded before the sequence is formed.
#define PLUS +
#define TWICE(x) x x
static_assert (^^{ a PLUS b } == ^^{ a + b });
static_assert (^^{ TWICE(z) } == ^^{ z z });

// A token sequence is a structural value: it can be a template argument.
template <info T> struct X { static constexpr info value = T; };
static_assert (X<^^{ a b }>::value == ^^{ a b });
static_assert (__is_same (X<^^{ a b }>, X<^^{ a  b }>));
static_assert (!__is_same (X<^^{ a b }>, X<^^{ a c }>));
}  // namespace equality

namespace in_templates {
template <typename T>
consteval info f () { return ^^{ T x; }; }
static_assert (f<int> () == ^^{ T x; });
static_assert (f<int> () == f<long> ());

template <int N>
constexpr info v = ^^{ N };
static_assert (v<1> == v<2>);
}  // namespace in_templates

namespace ill_formed_literals {
constexpr auto t1 = ^^{ a \ b };  // { dg-error "expected an interpolator" }

int x = \val(1);  // { dg-error "stray '\\\\' in program" }
}  // namespace ill_formed_literals

constexpr auto unterminated = ^^{ abc { def };  // { dg-error "expected '\}' at end of token sequence" }
// { dg-message "token sequence starts here" "" { target *-*-* } .-1 }
// { dg-error "expected ',' or ';' at end of input" "" { target *-*-* } .-2 }
