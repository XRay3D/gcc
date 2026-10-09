// std::meta::definition_tokens_of returns the tokens of the definition of a
// function (its body), a class (its member-specification, including the
// bodies of member functions), or a namespace (the declarations of all its
// definitions).  Adapted from EDG's tests/tests/reflections/tokens_b.sft.cpp.
// { dg-do run { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection -fdefinition-tokens" }

#include <meta>
#include <cstdio>

using namespace std::meta;

int square (int x) { return x * x; }
struct Point {
  int x, y;
  int sum () const { return x + y; }
  struct Inner { int z; };
};
namespace geo { int one () { return 1; } }
namespace geo { int two () { return 2; } }
template <class T> T twice (T v) { return v + v; }
template <class T> struct Box { T value; T get () const { return value; } };
struct Holder {
  // A consteval block and a range-based for in the recorded class body.
  consteval { for (int i : {1, 2}) (void) i; }
  int h () { return 5; }
};

[[edg::retain_tokens]] int marked (int a) { if (a) return 1; else return 0; }
struct [[edg::retain_tokens]] S { int v; int get () const { return v; } };
namespace [[edg::retain_tokens]] N { int f () { return 7; } }

#define SHOW(r) std::printf ("%-16s: %s\n", #r, \
    std::define_static_string (token_spelling_of (definition_tokens_of (r))))

int main ()
{
  SHOW (^^square);
  SHOW (^^Point);
  SHOW (^^Point::sum);
  SHOW (^^Point::Inner);
  SHOW (^^geo);
  SHOW (^^geo::two);
  SHOW (^^Holder);
  SHOW (^^Holder::h);
  SHOW (^^twice);
  SHOW (^^Box);
  SHOW (^^Box<int>);
  SHOW (^^Box<int>::get);
  SHOW (^^marked);
  SHOW (^^S);
  SHOW (^^S::get);
  SHOW (^^N);
  SHOW (^^N::f);
}

// { dg-output "\\^\\^square        : \\{ return x \\* x ; \\}\n" }
// { dg-output "\\^\\^Point         : int x , y ; int sum \\( \\) const \\{ return x \\+ y ; \\} struct Inner \\{ int z ; \\} ;\n" }
// { dg-output "\\^\\^Point::sum    : \\{ return x \\+ y ; \\}\n" }
// { dg-output "\\^\\^Point::Inner  : int z ;\n" }
// { dg-output "\\^\\^geo           : int one \\( \\) \\{ return 1 ; \\} int two \\( \\) \\{ return 2 ; \\}\n" }
// { dg-output "\\^\\^geo::two      : \\{ return 2 ; \\}\n" }
// { dg-output "\\^\\^Holder        : consteval \\{ for \\( int i : \\{ 1 , 2 \\} \\) \\( void \\) i ; \\} int h \\( \\) \\{ return 5 ; \\}\n" }
// { dg-output "\\^\\^Holder::h     : \\{ return 5 ; \\}\n" }
// { dg-output "\\^\\^twice         : \\{ return v \\+ v ; \\}\n" }
// { dg-output "\\^\\^Box           : T value ; T get \\( \\) const \\{ return value ; \\}\n" }
// { dg-output "\\^\\^Box<int>      : T value ; T get \\( \\) const \\{ return value ; \\}\n" }
// { dg-output "\\^\\^Box<int>::get : \\{ return value ; \\}\n" }
// { dg-output "\\^\\^marked        : \\{ if \\( a \\) return 1 ; else return 0 ; \\}\n" }
// { dg-output "\\^\\^S             : int v ; int get \\( \\) const \\{ return v ; \\}\n" }
// { dg-output "\\^\\^S::get        : \\{ return v ; \\}\n" }
// { dg-output "\\^\\^N             : int f \\( \\) \\{ return 7 ; \\}\n" }
// { dg-output "\\^\\^N::f          : \\{ return 7 ; \\}\n" }
