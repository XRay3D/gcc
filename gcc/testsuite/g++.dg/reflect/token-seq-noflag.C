// Token sequences need -ftoken-injection.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection" }

#ifdef __GXX_TOKEN_INJECTION__
#error "__GXX_TOKEN_INJECTION__ defined"
#endif
static_assert (!__has_feature (token_injection));

constexpr auto t = ^^{ int x; };  // { dg-error "token sequences are only available with '-ftoken-injection'" }
