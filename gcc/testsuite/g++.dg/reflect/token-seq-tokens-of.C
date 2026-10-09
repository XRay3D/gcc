// Iteration over the tokens of a token sequence: std::meta::tokens_of and
// std::meta::token_kind_of (an extension to P3294, as in EDG and
// clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

namespace kinds {
constexpr auto t = ^^{ x int :: bitor 0x10 2.5 'a' "s" true false this nullptr };
static_assert (tokens_of (t).size () == 12);
static_assert (token_kind_of (tokens_of (t)[0]) == token_kind::identifier);
static_assert (token_kind_of (tokens_of (t)[1]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (t)[2]) == token_kind::punctuator);
static_assert (token_kind_of (tokens_of (t)[3]) == token_kind::punctuator);
static_assert (token_kind_of (tokens_of (t)[4]) == token_kind::integer_literal);
static_assert (token_kind_of (tokens_of (t)[5]) == token_kind::floating_literal);
static_assert (token_kind_of (tokens_of (t)[6]) == token_kind::character_literal);
static_assert (token_kind_of (tokens_of (t)[7]) == token_kind::string_literal);
static_assert (token_kind_of (tokens_of (t)[8]) == token_kind::boolean_literal);
static_assert (token_kind_of (tokens_of (t)[9]) == token_kind::boolean_literal);
static_assert (token_kind_of (tokens_of (t)[10]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (t)[11]) == token_kind::keyword);

// The kind is that of the first token.
static_assert (token_kind_of (t) == token_kind::identifier);
static_assert (token_kind_of (^^{ 1 + 2 }) == token_kind::integer_literal);

// Other literal forms.
constexpr auto lits = ^^{ 1e3 0x1p3 1'000 u8'a' L"s" R"(x)" u8"s" 42_x 2.5_x
			  "s"_x 'c'_x };
static_assert (tokens_of (lits).size () == 11);
static_assert (token_kind_of (tokens_of (lits)[0]) == token_kind::floating_literal);
static_assert (token_kind_of (tokens_of (lits)[1]) == token_kind::floating_literal);
static_assert (token_kind_of (tokens_of (lits)[2]) == token_kind::integer_literal);
static_assert (token_kind_of (tokens_of (lits)[3]) == token_kind::character_literal);
static_assert (token_kind_of (tokens_of (lits)[4]) == token_kind::string_literal);
static_assert (token_kind_of (tokens_of (lits)[5]) == token_kind::string_literal);
static_assert (token_kind_of (tokens_of (lits)[6]) == token_kind::string_literal);
static_assert (token_kind_of (tokens_of (lits)[7]) == token_kind::user_defined_literal);
static_assert (token_kind_of (tokens_of (lits)[8]) == token_kind::user_defined_literal);
static_assert (token_kind_of (tokens_of (lits)[9]) == token_kind::user_defined_literal);
static_assert (token_kind_of (tokens_of (lits)[10]) == token_kind::user_defined_literal);

// Interpolated tokens.
constexpr info r = ^^int;
constexpr const char *name = "a";
constexpr auto interp = ^^{ \val(42) \[: r :] \str("hi") \[name, 1] \{t} };
static_assert (tokens_of (interp).size () == 1 + 3 + 1 + 1 + 12);
static_assert (token_kind_of (tokens_of (interp)[0]) == token_kind::interpolated);
static_assert (token_kind_of (tokens_of (interp)[1]) == token_kind::punctuator);
static_assert (token_kind_of (tokens_of (interp)[2]) == token_kind::interpolated);
static_assert (token_kind_of (tokens_of (interp)[3]) == token_kind::punctuator);
static_assert (token_kind_of (tokens_of (interp)[4]) == token_kind::string_literal);
static_assert (token_kind_of (tokens_of (interp)[5]) == token_kind::identifier);
static_assert (token_kind_of (tokens_of (interp)[6]) == token_kind::identifier);

// Each element is a token sequence holding one token.
static_assert (is_token_sequence (tokens_of (t)[0]));
static_assert (tokens_of (tokens_of (t)[0]).size () == 1);
static_assert (tokens_of (t)[0] == ^^{ x });
static_assert (tokens_of (t)[2] == ^^{ :: });
static_assert (tokens_of (^^{}).empty ());
}  // namespace kinds

namespace reassembly {
// A token sequence is concatenated by interpolating it into a new one.
consteval info rebuild (info s)
{
  info out = ^^{};
  for (info t : tokens_of (s))
    out = ^^{ \{out} \{t} };
  return out;
}

consteval info rebuild_with_list_builder (info s)
{
  list_builder out (^^{});
  for (info t : tokens_of (s))
    out += t;
  return out;
}

// Rebuild, replacing each identifier "x" by "y".
consteval info rename_x (info s)
{
  info out = ^^{};
  for (info t : tokens_of (s))
    if (token_kind_of (t) == token_kind::identifier && t == ^^{ x })
      out = ^^{ \{out} y };
    else
      out = ^^{ \{out} \{t} };
  return out;
}

constexpr info seq = ^^{ int x = 0x10 + 'a';
  if (x > 3) f ("s", 2.5, true); };
static_assert (rebuild (seq) == seq);
static_assert (rebuild_with_list_builder (seq) == seq);
static_assert (tokens_of (rebuild (seq)).size () == tokens_of (seq).size ());
static_assert (rename_x (^^{ x + x * z }) == ^^{ y + y * z });

// A function injected from reassembled (and edited) tokens.
consteval {
  queue_injection (rename_x (^^{ constexpr int answer () { int x = 6; return x * 7; } }));
}
static_assert (answer () == 42);

// A lone brace taken from a token sequence: injected tokens that close the
// scope they are injected into are diagnosed.
constexpr info rbrace = tokens_of (^^{ { } })[1];  // { dg-line rbrace_line }
struct S {
  consteval { queue_injection (^^{ int a; \{rbrace} int b; }); }  // { dg-message "in token sequence injected here" }
  // { dg-error "extraneous tokens in token sequence injected as member declarations" "" { target *-*-* } rbrace_line }
};
void f ()
{
  consteval { queue_injection (^^{ ; \{rbrace} ; }); }  // { dg-message "in token sequence injected here" }
  // { dg-error "extraneous tokens in token sequence injected as statements" "" { target *-*-* } rbrace_line }
}
consteval { namespace_inject (^^{ int c; \{rbrace} int d; }); }  // { dg-message "in token sequence injected here" }
// { dg-error "extraneous tokens in token sequence injected as declarations" "" { target *-*-* } rbrace_line }
}  // namespace reassembly

namespace errors {
constexpr auto n1 = tokens_of (^^int).size ();  // { dg-error "uncaught exception of type 'std::meta::exception'; 'what\\(\\)': 'reflection does not represent a token sequence'" }
constexpr token_kind k1 = token_kind_of (^^{});  // { dg-error "uncaught exception of type 'std::meta::exception'; 'what\\(\\)': 'token sequence is empty'" }
constexpr token_kind k2 = token_kind_of (^^::);  // { dg-error "uncaught exception of type 'std::meta::exception'; 'what\\(\\)': 'reflection does not represent a token sequence'" }
}  // namespace errors
