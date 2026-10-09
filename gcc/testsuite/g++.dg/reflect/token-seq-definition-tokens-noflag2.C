// The retain_tokens attribute is ignored without -ftoken-injection.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection" }

[[gnu::retain_tokens]] int f () { return 0; }  // { dg-warning "'retain_tokens' attribute ignored without '-ftoken-injection'" }
struct [[edg::retain_tokens]] S { };  // { dg-warning "'retain_tokens' attribute ignored without '-ftoken-injection'" }
namespace [[gnu::retain_tokens]] N { }  // { dg-warning "'retain_tokens' attribute ignored without '-ftoken-injection'" }
