// std::meta::definition_tokens_of with -fdefinition-tokens (an extension to
// P3294, as in EDG and clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection -fdefinition-tokens" }

#include <meta>
#include <string_view>

using namespace std::meta;

#define TOKENS(r) std::string_view (token_spelling_of (definition_tokens_of (r)))

static_assert (__has_feature (definition_tokens));
static_assert (__has_cpp_attribute (gnu::retain_tokens));
static_assert (__has_cpp_attribute (edg::retain_tokens));

int square (int x) { return x * x; }

struct Point {
  int x, y;
  int sum () const { return x + y; }
  struct Inner { int z; };
};

namespace geo { int one () { return 1; } }
namespace geo { int two () { return 2; } }

namespace a::b { int c () { return 3; } }

struct Ctor {
  int v;
  Ctor (int a) : v (a) { v++; }
  Ctor () try : v (0) { } catch (...) { }
};

int tryf () try { return 1; } catch (...) { return 2; }

int out_of_line ();
int out_of_line () { return 4; }

struct Holder {
  consteval { }
  int h () { return 5; }
};

struct Brace {
  int a, b;
  Brace () : a{1}, b (2) { }
};

template <class T> T twice (T v) { return v + v; }
template <> char twice<char> (char v) { return v; }

template <class T> struct Box {
  T value;
  T get () const { return value; }
};
template <class T> struct Box<T *> { T *p; };

static_assert (TOKENS (^^square) == "{ return x * x ; }");
static_assert (TOKENS (^^Point)
	       == "int x , y ; int sum ( ) const { return x + y ; } "
		  "struct Inner { int z ; } ;");
static_assert (TOKENS (^^Point::sum) == "{ return x + y ; }");
static_assert (TOKENS (^^Point::Inner) == "int z ;");
static_assert (TOKENS (^^geo)
	       == "int one ( ) { return 1 ; } int two ( ) { return 2 ; }");
static_assert (TOKENS (^^geo::two) == "{ return 2 ; }");
static_assert (TOKENS (^^a::b) == "int c ( ) { return 3 ; }");
static_assert (TOKENS (^^Ctor)
	       == "int v ; Ctor ( int a ) : v ( a ) { v ++ ; } "
		  "Ctor ( ) try : v ( 0 ) { } catch ( ... ) { }");
static_assert (TOKENS (^^tryf) == "try { return 1 ; } catch ( ... ) { return 2 ; }");
static_assert (TOKENS (^^out_of_line) == "{ return 4 ; }");
static_assert (TOKENS (^^Holder) == "consteval { } int h ( ) { return 5 ; }");
static_assert (TOKENS (^^Holder::h) == "{ return 5 ; }");
static_assert (TOKENS (^^Brace) == "int a , b ; Brace ( ) : a { 1 } , b ( 2 ) { }");

// Templates and their specializations: the tokens of the template.
static_assert (TOKENS (^^twice) == "{ return v + v ; }");
static_assert (TOKENS (^^twice<int>) == "{ return v + v ; }");
static_assert (TOKENS (^^twice<char>) == "{ return v ; }");
static_assert (TOKENS (^^Box) == "T value ; T get ( ) const { return value ; }");
static_assert (TOKENS (^^Box<int>) == "T value ; T get ( ) const { return value ; }");
static_assert (TOKENS (^^Box<int>::get) == "{ return value ; }");
static_assert (TOKENS (^^Box<int *>) == "T * p ;");

// The tokens of a definition that is being used.
constexpr Box<long> b{42};
static_assert (TOKENS (^^Box<long>::get) == "{ return value ; }");

// Each call returns the same tokens.
static_assert (definition_tokens_of (^^square) == definition_tokens_of (^^square));

// The tokens are located in the original definition.
static_assert (token_location_of (definition_tokens_of (^^square)).line ()
	       == source_location_of (^^square).line ());

// The recorded tokens can be injected.
consteval {
  queue_injection (^^{ constexpr int square_copy (int x) \{definition_tokens_of (^^square)} });
}
static_assert (square_copy (7) == 49);

namespace errors {
int declared_only (int);
int v = 0;

constexpr auto e1 = definition_tokens_of (^^declared_only);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e2 = definition_tokens_of (^^int);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e3 = definition_tokens_of (^^{ x });  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e4 = definition_tokens_of (^^::);  // { dg-error "'the tokens of the definition are not available" }
constexpr auto e5 = definition_tokens_of (^^v);  // { dg-error "'the tokens of the definition are not available" }
}  // namespace errors
