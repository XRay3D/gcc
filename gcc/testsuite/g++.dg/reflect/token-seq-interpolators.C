// P3294 token sequences: the interpolators \[...], \[: :], \{...},
// \val(...) and \str(...).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <string_view>

using namespace std::meta;
using sv = std::string_view;

namespace identifier_interpolator {
constexpr sv name = "x";
constexpr int idx = 5;

static_assert (^^{ int \[name, idx]; } == ^^{ int x5; });
static_assert (^^{ \["abc"] } == ^^{ abc });
static_assert (^^{ \["a", "b", "c"] } == ^^{ abc });
static_assert (^^{ \["a", 1, "b", 23] } == ^^{ a1b23 });
static_assert (^^{ \[sv("hello world", 5)] } == ^^{ hello });
static_assert (^^{ \["_", 0u, 1L, 2ull] } == ^^{ _012 });
static_assert (^^{ \[u8"utf8"] } == ^^{ utf8 });
static_assert (^^{ \["n", -1] } != ^^{ n1 });  // { dg-error "'n-1' does not spell an identifier" }
// { dg-error "non-constant condition" "" { target *-*-* } .-1 }

// Operands are ordinary expressions of the enclosing context.
consteval info make (sv prefix, int n)
{
  info result = ^^{};
  for (int i = 0; i < n; ++i)
    result = ^^{ \{result} \[prefix, i] };
  return result;
}
static_assert (make ("v", 3) == ^^{ v0 v1 v2 });

// The operand of an identifier interpolator is only evaluated once.
consteval info once ()
{
  const char *names[] = {"first", "second"};
  int i = 0;
  info r = ^^{ \[sv(names[i++])] };
  return i == 1 ? r : ^^{};
}
static_assert (once () == ^^{ first });

constexpr auto bad1 = ^^{ \["1abc"] };  // { dg-error "'1abc' does not spell an identifier" }
constexpr auto bad2 = ^^{ \[""] };  // { dg-error "'' does not spell an identifier" }
constexpr auto bad3 = ^^{ \["while"] };  // { dg-error "'while' is a keyword, not an identifier" }
constexpr auto bad4 = ^^{ \["a b"] };  // { dg-error "'a b' does not spell an identifier" }

constexpr auto bad5 = ^^{ \[1, "a"] };  // { dg-error "first operand of '.\\\[' interpolator must be a string, but has type 'int'" }
constexpr auto bad6 = ^^{ \["a", 1.5] };  // { dg-error "operand 2 of '.\\\[' interpolator must be a string or an integer, but has type 'double'" }
struct NotAString { int size () const; };
constexpr auto bad7 = ^^{ \[NotAString{}] };  // { dg-error "class type 'identifier_interpolator::NotAString' lacks a 'size\\(\\)' or 'data\\(\\)' member" }
constexpr auto bad8 = ^^{ \[] };  // { dg-error "expected primary-expression" }
constexpr auto bad9 = ^^{ \["a" };  // { dg-error "expected '\]'" }
}  // namespace identifier_interpolator

namespace tokens_interpolator {
constexpr auto inner = ^^{ a + b };
constexpr auto outer = ^^{ return \{inner}; };
static_assert (outer == ^^{ return a + b; });
static_assert (^^{ \{^^{}} } == ^^{});
static_assert (^^{ ( \{inner} ) * \{inner} } == ^^{ ( a + b ) * a + b });

// A class type operand is converted to 'info', even explicitly.
struct Wrapper {
  info tokens;
  consteval explicit operator info () const { return tokens; }
};
static_assert (^^{ [ \{Wrapper{inner}} ] } == ^^{ [ a + b ] });

consteval info join ()
{
  list_builder args;
  args += ^^{ int a };
  args += ^^{};
  args += ^^{ int b };
  list_builder sum (^^{ + });
  sum += ^^{ a };
  sum += ^^{ b };
  return ^^{ int f(\{args}) { return \{sum}; } };
}
static_assert (join () == ^^{ int f(int a, int b) { return a + b; } });

constexpr auto bad1 = ^^{ \{^^int} };  // { dg-error "operand of '.\\\{...\\\}' interpolator is not a token sequence" }
constexpr auto bad2 = ^^{ \{42} };  // { dg-error "could not convert" }
}  // namespace tokens_interpolator

namespace value_interpolator {
constexpr int val = 42;
static_assert (^^{ int x = \val(val); } == ^^{ int x = \val(42); });
static_assert (^^{ \val(1) } != ^^{ \val(2) });
static_assert (^^{ \val(1) } != ^^{ \val(1L) });
// A value is not the same token as the literal spelling it.
static_assert (^^{ \val(1) } != ^^{ 1 });
// A reflection is a value too.
static_assert (^^{ \val(^^int) } == ^^{ \val(^^int) });
static_assert (^^{ \val(^^int) } != ^^{ \val(^^long) });
// So is an object of a structural class type.
struct P { int a, b; };
static_assert (^^{ \val(P{1, 2}) } == ^^{ \val(P{1, 2}) });
static_assert (^^{ \val(P{1, 2}) } != ^^{ \val(P{2, 1}) });

struct NonStructural { private: int x = 0; public: constexpr NonStructural () {} };
constexpr auto bad1 = ^^{ \val(NonStructural{}) };  // { dg-error "operand of '.val' interpolator has type 'value_interpolator::NonStructural', which is not a structural type" }
constexpr auto bad2 = ^^{ \val() };  // { dg-error "expected primary-expression" }
}  // namespace value_interpolator

namespace string_interpolator {
constexpr sv text = "hello";
static_assert (^^{ \str(text) } == ^^{ \str("hello") });
static_assert (^^{ \str("a") } != ^^{ \str("b") });
static_assert (^^{ \str(sv("hello world", 5)) } == ^^{ \str(text) });
static_assert (^^{ \str("quote\"back\\slash\n") } == ^^{ \str("quote\"back\\slash\n") });

constexpr auto bad1 = ^^{ \str(42) };  // { dg-error "operand of '.str' interpolator must be a string, but has type 'int'" }
}  // namespace string_interpolator

namespace splice_interpolator {
static_assert (^^{ \[:^^int:] } == ^^{ \[:^^int:] });
static_assert (^^{ \[:^^int:] } != ^^{ \[:^^long:] });
static_assert (^^{ \[:^^int:] } != ^^{ int });

constexpr auto bad1 = ^^{ \[: 42 :] };  // { dg-error "could not convert" }
constexpr auto bad2 = ^^{ \[: ^^int ] };  // { dg-error "expected ':\]'" }
}  // namespace splice_interpolator

namespace dependent_operands {
template <typename T, int N>
consteval info make ()
{
  return ^^{ \[:^^T:] \["arr", N][\val(N)]; };
}
static_assert (make<int, 3> () == ^^{ \[:^^int:] arr3[\val(3)]; });
static_assert (make<char, 4> () != make<char, 5> ());

template <typename S>
consteval info named (S s) { return ^^{ \[s] }; }
static_assert (named ("lit") == ^^{ lit });
static_assert (named (sv ("view")) == ^^{ view });

template <typename T>
consteval info bad (T t) { return ^^{ \[t] }; }  // { dg-error "first operand of '.\\\[' interpolator must be a string, but has type 'double'" }
constexpr auto b = bad (1.5);  // { dg-message "required from here" }
}  // namespace dependent_operands
