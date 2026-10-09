// Iterate over the tokens of a token sequence (std::meta::tokens_of),
// logging the kind, spelling, value, and line of each token, and reassemble
// the tokens into an equivalent token sequence.  Adapted from EDG's
// tests/tests/reflections/tokens_a.sft.cpp.
// { dg-do run { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

using namespace std::meta;

consteval const char *kind_name (token_kind k)
{
  switch (k)
    {
    case token_kind::identifier: return "identifier";
    case token_kind::keyword: return "keyword";
    case token_kind::punctuator: return "punctuator";
    case token_kind::integer_literal: return "integer";
    case token_kind::floating_literal: return "floating";
    case token_kind::character_literal: return "character";
    case token_kind::string_literal: return "string";
    case token_kind::boolean_literal: return "boolean";
    case token_kind::user_defined_literal: return "udl";
    case token_kind::interpolated: return "interpolated";
    case token_kind::operator_function_id: return "operator-function-id";
    }
  return "?";
}

consteval std::string number (unsigned long n)
{
  std::string s;
  do { s.insert (s.begin (), char ('0' + n % 10)); n /= 10; } while (n != 0);
  return s;
}

consteval bool is_literal (token_kind k)
{
  return (k != token_kind::identifier && k != token_kind::keyword
	  && k != token_kind::punctuator && k != token_kind::interpolated);
}

// One line per token: "kind 'spelling' line N [value=V]".
consteval std::string describe (info seq)
{
  std::string out;
  for (info t : tokens_of (seq))
    {
      token_kind k = token_kind_of (t);
      out += kind_name (k);
      out += " '";
      out += token_spelling_of (t);
      out += "' line ";
      out += number (token_location_of (t).line () - token_location_of (seq).line ());
      if (is_literal (k) && k != token_kind::string_literal)
	{
	  out += " value=";
	  out += display_string_of (token_value_of (t));
	}
      out += "\n";
    }
  return out;
}

constexpr info seq = ^^{ int x = 0x10 + 'a';
  if (x > 3) f("s", 7, true); };

// Reassemble the tokens one at a time by interpolation.
consteval info rebuild (info s)
{
  info out = ^^{};
  for (info t : tokens_of (s))
    out = ^^{ \{out} \{t} };
  return out;
}

// The same with std::meta::list_builder (which interpolates as well).
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
    {
      if (token_kind_of (t) == token_kind::identifier
	  && token_spelling_of (t) == "x")
	out = ^^{ \{out} y };
      else
	out = ^^{ \{out} \{t} };
    }
  return out;
}

static_assert (token_spelling_of (rebuild (seq)) == token_spelling_of (seq));
static_assert (tokens_of (rebuild (seq)).size () == tokens_of (seq).size ());
static_assert (token_spelling_of (rebuild_with_list_builder (seq))
	       == token_spelling_of (seq));
static_assert (token_spelling_of (rename_x (^^{ x + x * z })) == "y + y * z");
static_assert (tokens_of (^^{}).empty ());

// The standard queries display_string_of and source_location_of also work on
// token sequences: they give the text and the position of the tokens.
consteval bool standard_queries_agree (info s)
{
  for (info t : tokens_of (s))
    {
      if (display_string_of (t) != token_spelling_of (t))
	return false;
      if (source_location_of (t).line () != token_location_of (t).line ())
	return false;
      if (source_location_of (t).column () != token_location_of (t).column ())
	return false;
    }
  return display_string_of (s) == token_spelling_of (s);
}
static_assert (standard_queries_agree (seq));

constexpr int limit = 42;
static_assert ([: token_value_of (tokens_of (^^{ 0x10 })[0]) :] == 16);
static_assert (token_value_of (tokens_of (^^{ 'a' })[0]) == reflect_constant ('a'));
constexpr info val_limit = tokens_of (^^{ \val(^^limit) })[0];
static_assert (token_kind_of (val_limit) == token_kind::interpolated);
static_assert (token_value_of (val_limit) == ^^limit);
static_assert ([: token_value_of (val_limit) :] == 42);
constexpr info val_42 = tokens_of (^^{ \val(42) })[0];
static_assert (token_kind_of (val_42) == token_kind::interpolated);
static_assert (token_value_of (val_42) == reflect_constant (42));
static_assert (token_kind_of (tokens_of (^^{ \[:^^limit:] })[1])
	       == token_kind::interpolated);

// An identifier has no value; the entity it names is found by injecting it.
consteval {
  queue_injection (^^{ constexpr info found_limit =
			 ^^\{tokens_of (^^{ limit })[0]}; });
}
static_assert (found_limit == ^^limit);

// A function injected from reassembled (and edited) tokens.
consteval {
  queue_injection (rename_x (^^{ int answer () { int x = 6; return x * 7; } }));
}

int main ()
{
  std::fputs (std::define_static_string (describe (seq)), stdout);
  constexpr std::size_t count = tokens_of (seq).size ();
  std::printf ("count=%zu\n", count);
  std::printf ("rebuilt: %s\n",
	       std::define_static_string (token_spelling_of (rename_x (seq))));
  std::printf ("answer()=%d\n", answer ());
}

// { dg-output "keyword 'int' line 0\n" }
// { dg-output "identifier 'x' line 0\n" }
// { dg-output "punctuator '=' line 0\n" }
// { dg-output "integer '0x10' line 0 value=16\n" }
// { dg-output "punctuator '\\+' line 0\n" }
// { dg-output "character ''a'' line 0 value='a'\n" }
// { dg-output "punctuator ';' line 0\n" }
// { dg-output "keyword 'if' line 1\n" }
// { dg-output "punctuator '\\(' line 1\n" }
// { dg-output "identifier 'x' line 1\n" }
// { dg-output "punctuator '>' line 1\n" }
// { dg-output "integer '3' line 1 value=3\n" }
// { dg-output "punctuator '\\)' line 1\n" }
// { dg-output "identifier 'f' line 1\n" }
// { dg-output "punctuator '\\(' line 1\n" }
// { dg-output "string '\"s\"' line 1\n" }
// { dg-output "punctuator ',' line 1\n" }
// { dg-output "integer '7' line 1 value=7\n" }
// { dg-output "punctuator ',' line 1\n" }
// { dg-output "boolean 'true' line 1 value=true\n" }
// { dg-output "punctuator '\\)' line 1\n" }
// { dg-output "punctuator ';' line 1\n" }
// { dg-output "count=22\n" }
// { dg-output "rebuilt: int y = 0x10 \\+ 'a' ; if \\( y > 3 \\) f \\( \"s\" , 7 , true \\) ;\n" }
// { dg-output "answer\\(\\)=42\n" }
