// std::meta::token_location_of and source_location_of of a token sequence
// (an extension to P3294, as in EDG and clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <string_view>

using namespace std::meta;

constexpr auto t = ^^{ a
  b };
static_assert (token_location_of (tokens_of (t)[0]).line () == __LINE__ - 2);
static_assert (token_location_of (tokens_of (t)[1]).line () == __LINE__ - 2);
static_assert (token_location_of (tokens_of (t)[0]).column () == 24);
static_assert (token_location_of (tokens_of (t)[1]).column () == 3);
static_assert (token_location_of (t).line () == token_location_of (tokens_of (t)[0]).line ());
static_assert (std::string_view (token_location_of (t).file_name ()).ends_with ("token-seq-location.C"));

// A token from a macro expansion is located at the expansion.
#define TOKS ^^{ x y }
constexpr auto m = TOKS;
static_assert (token_location_of (m).line () == __LINE__ - 1);

// An interpolated token is located at its interpolator.
constexpr auto i = ^^{ a
		       \val(1) };
static_assert (token_location_of (tokens_of (i)[1]).line () == __LINE__ - 1);

// Tokens keep their location when they are copied by interpolation.
constexpr auto c = ^^{ z \{t} };
static_assert (token_location_of (tokens_of (c)[1]).line () == token_location_of (t).line ());

// source_location_of gives the same location, and an empty source_location
// for an empty sequence.
consteval bool same_locations (info s)
{
  for (info tok : tokens_of (s))
    if (source_location_of (tok).line () != token_location_of (tok).line ()
	|| source_location_of (tok).column () != token_location_of (tok).column ())
      return false;
  return true;
}
static_assert (same_locations (t) && same_locations (i) && same_locations (c));
static_assert (source_location_of (^^{}).line () == 0);

constexpr auto l1 = token_location_of (^^{});  // { dg-error "'token sequence is empty'" }
constexpr auto l2 = token_location_of (^^int);  // { dg-error "'reflection does not represent a token sequence'" }
