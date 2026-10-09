// std::meta::definition_tokens_of with the retain_tokens attribute (an
// extension to P3294, as in EDG and clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <string_view>

using namespace std::meta;

#define TOKENS(r) std::string_view (token_spelling_of (definition_tokens_of (r)))

static_assert (!__has_feature (definition_tokens));

[[gnu::retain_tokens]] int marked (int a) { if (a) return 1; else return 0; }

struct [[edg::retain_tokens]] S {
  int v;
  int get () const { return v; }
};

namespace [[gnu::retain_tokens]] N { int f () { return 7; } }

// The attribute of a prior declaration is inherited by the definition.
[[gnu::retain_tokens]] int declared_first (int);
int declared_first (int x) { return x; }

struct [[gnu::retain_tokens]] S2;
struct S2 { int w; };

// Definitions nested in a recorded one are recorded too.
struct [[gnu::retain_tokens]] Outer {
  template <class T> T m (T v) { return v; }
  struct Nested {
    int q () { return 9; }
  };
  int d = 1;
};

namespace [[gnu::retain_tokens]] NS {
struct In { int r () { return 10; } };
namespace deeper { int s () { return 11; } }
}

// A macro-expanded body is recorded as expanded.
#define BODY { return 12; }
#define RET(x) return x
[[gnu::retain_tokens]] int macro_body () BODY
[[gnu::retain_tokens]] int macro_in_body () { RET(13); }

// Class heads with lookahead before the body.
struct [[gnu::retain_tokens]] Final final { int v; };
struct [[gnu::retain_tokens]] Derived : S { int w; };

// An in-class defined member function with the attribute, in a class that
// is not recorded.
struct Plain {
  [[gnu::retain_tokens]] int get () const { return 14; }
  int other () const { return 15; }
};

// The tokens of templates are always recorded.
template <class T> T twice (T v) { return v + v; }
template <class T> struct TBox { T value; };
namespace plain { int f () { return 0; } }

static_assert (TOKENS (^^marked) == "{ if ( a ) return 1 ; else return 0 ; }");
static_assert (TOKENS (^^S) == "int v ; int get ( ) const { return v ; }");
static_assert (TOKENS (^^S::get) == "{ return v ; }");
static_assert (TOKENS (^^N) == "int f ( ) { return 7 ; }");
static_assert (TOKENS (^^N::f) == "{ return 7 ; }");
static_assert (TOKENS (^^declared_first) == "{ return x ; }");
static_assert (TOKENS (^^S2) == "int w ;");
static_assert (TOKENS (^^Outer)
	       == "template < class T > T m ( T v ) { return v ; } "
		  "struct Nested { int q ( ) { return 9 ; } } ; int d = 1 ;");
static_assert (TOKENS (^^Outer::m) == "{ return v ; }");
static_assert (TOKENS (^^Outer::Nested) == "int q ( ) { return 9 ; }");
static_assert (TOKENS (^^Outer::Nested::q) == "{ return 9 ; }");
static_assert (TOKENS (^^NS)
	       == "struct In { int r ( ) { return 10 ; } } ; "
		  "namespace deeper { int s ( ) { return 11 ; } }");
static_assert (TOKENS (^^NS::In::r) == "{ return 10 ; }");
static_assert (TOKENS (^^NS::deeper) == "int s ( ) { return 11 ; }");
static_assert (TOKENS (^^NS::deeper::s) == "{ return 11 ; }");
static_assert (TOKENS (^^macro_body) == "{ return 12 ; }");
static_assert (TOKENS (^^macro_in_body) == "{ return 13 ; }");
static_assert (TOKENS (^^Final) == "int v ;");
static_assert (TOKENS (^^Derived) == "int w ;");
static_assert (TOKENS (^^Plain::get) == "{ return 14 ; }");
static_assert (TOKENS (^^twice) == "{ return v + v ; }");
static_assert (TOKENS (^^twice<int>) == "{ return v + v ; }");
static_assert (TOKENS (^^TBox) == "T value ;");
static_assert (TOKENS (^^TBox<int>) == "T value ;");

constexpr auto e1 = definition_tokens_of (^^Plain);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e2 = definition_tokens_of (^^Plain::other);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e3 = definition_tokens_of (^^plain);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e4 = definition_tokens_of (^^plain::f);  // { dg-error "'the tokens of the definition are not available" }

// The recorded tokens can be injected.
consteval {
  queue_injection (^^{ constexpr int marked_copy (int a) \{definition_tokens_of (^^marked)} });
}
static_assert (marked_copy (1) == 1 && marked_copy (0) == 0);

[[gnu::retain_tokens]] int var = 0;  // { dg-warning "'retain_tokens' attribute only applies to functions, classes and namespaces" }
