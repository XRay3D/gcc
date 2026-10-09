// std::meta::__report_tokens prints a token sequence.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

consteval { __report_tokens (^^{ int   x = 42 ; }); }  // { dg-warning "token sequence: int x = 42 ;" }
consteval { __report_tokens (^^{}); }  // { dg-warning "token sequence:" }
consteval { __report_tokens (^^{ f("a b", 'c', 1.5e3) -> x }); }  // { dg-warning "token sequence: f \\( \"a b\" , 'c' , 1.5e3 \\) -> x" }
consteval { __report_tokens (^^{ \[:^^int:] \["id", 1] = \val(42) + \str("s"); }); }  // { dg-warning "token sequence: \\\[: \\^\\^int :\\\] id1 = 42 \\+ \"s\" ;" }
consteval { __report_tokens (^^{ a bitor b and c }); }  // { dg-warning "token sequence: a bitor b and c" }

consteval { __report_tokens (^^int); }  // { dg-error "'std::meta::__report_tokens' called with a reflection that is not a token sequence" }
