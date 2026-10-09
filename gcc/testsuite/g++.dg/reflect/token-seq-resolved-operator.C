// std::meta::resolved_operator_of gives, for an operator token of
// a definition (from std::meta::definition_tokens_of), the operator function
// that overload resolution selected for it: a member function, a function
// found by argument-dependent lookup, or (in C++20) a rewritten comparison,
// possibly with reversed operands.  For a built-in operator, the result is
// the token itself.  Adapted from EDG's tests/tests/reflections/tokens_f.sft.cpp
// and tokens_e.sft.cpp.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <compare>
#include <cstdio>
#include <string>
#include <string_view>

using namespace std::meta;

struct S {
  int v;
  S operator+ (S o) const { return {v + o.v}; }
};
namespace ns {
  struct N { int v; };
  N operator+ (N a, N b) { return {a.v + b.v}; }
  bool operator== (N a, N b) { return a.v == b.v; }
  std::strong_ordering operator<=> (N a, int b) { return a.v <=> b; }
}
struct V {
  int a[4];
  int &operator[] (int i) { return a[i]; }
};
struct P {
  S s;
  S *operator-> () { return &s; }
};

[[gnu::retain_tokens]]
int f (int a, int b, S s1, S s2, ns::N n1, ns::N n2, V v, P p)
{
  int x = a + b;
  S s3 = s1 + s2;
  ns::N n3 = n1 + n2;
  bool ne = n1 != n2;
  bool lt = 1 < n1;
  v[1] = x;
  return p->v + s3.v + n3.v + ne + lt;
}

consteval std::string number (unsigned long n)
{
  std::string s;
  do { s.insert (s.begin (), char ('0' + n % 10)); n /= 10; } while (n != 0);
  return s;
}

// A token that, preceded by "operator", forms an operator-function-id.
consteval bool is_operator_token (info t)
{
  return token_kind_of (^^{ operator \{t} }) == token_kind::operator_function_id;
}

// One line per operator token of the body: the selected function (or
// "built-in") and how it was found.
consteval std::string describe (info body)
{
  std::string out;
  unsigned base = token_location_of (body).line ();
  for (info t : tokens_of (body))
    {
      if (!is_operator_token (t) && token_spelling_of (t) != "[")
	continue;
      info r = resolved_operator_of (t);
      out += "line ";
      out += number (token_location_of (t).line () - base);
      out += ": '";
      out += token_spelling_of (t);
      out += "' -> ";
      if (r == t)
	out += "built-in";
      else
	{
	  out += display_string_of (r);
	  out += " (operator_of: ";
	  out += symbol_of (operator_of (r));
	  out += ")";
	  if (is_resolved_through_adl (t)) out += " [ADL]";
	  if (is_rewritten_operator (t)) out += " [rewritten]";
	  if (has_reversed_operands (t)) out += " [reversed]";
	}
      out += "\n";
    }
  return out;
}

// The n-th token of the body of fn spelled sp.
consteval info nth_token (info fn, std::string_view sp, int n)
{
  for (info t : tokens_of (definition_tokens_of (fn)))
    if (token_spelling_of (t) == sp && n-- == 0)
      return t;
  return ^^{};
}

static_assert (parent_of (resolved_operator_of (nth_token (^^f, "+", 1))) == ^^S);
static_assert (parent_of (resolved_operator_of (nth_token (^^f, "+", 2))) == ^^ns);
static_assert (is_resolved_through_adl (nth_token (^^f, "+", 2)));
static_assert (!is_resolved_through_adl (nth_token (^^f, "+", 1)));
static_assert (operator_of (resolved_operator_of (nth_token (^^f, "!=", 0)))
	       == operators::op_equals_equals);
static_assert (is_rewritten_operator (nth_token (^^f, "!=", 0)));
static_assert (!has_reversed_operands (nth_token (^^f, "!=", 0)));
static_assert (has_reversed_operands (nth_token (^^f, "<", 0)));
static_assert (resolved_operator_of (nth_token (^^f, "<", 0)) == ^^ns::operator<=>);
static_assert (resolved_operator_of (nth_token (^^f, "[", 0)) == ^^V::operator[]);
static_assert (resolved_operator_of (nth_token (^^f, "->", 0)) == ^^P::operator->);

// For a built-in operator, the result is the token itself.
consteval bool is_built_in (info fn, std::string_view sp, int n)
{
  info t = nth_token (fn, sp, n);
  return resolved_operator_of (t) == t && !is_resolved_through_adl (t);
}
static_assert (is_built_in (^^f, "+", 0));
static_assert (!is_built_in (^^f, "+", 1));
// So it is for a token that is not an operator of an expression.
static_assert (is_built_in (^^f, "=", 0));
static_assert (is_built_in (^^f, ";", 0));
static_assert (is_built_in (^^f, "int", 0));

// The tokens keep their link to the definition when they are copied by
// interpolation (as in clang-p2996; EDG loses it).
static_assert (resolved_operator_of (^^{ \{nth_token (^^f, "+", 1)} x })
	       == resolved_operator_of (nth_token (^^f, "+", 1)));

// The predicates never throw.
static_assert (!is_resolved_through_adl (^^int));
static_assert (!is_rewritten_operator (^^{}));
static_assert (!has_reversed_operands (^^{ + }));

// In a template, an operator that does not depend on a template parameter is
// resolved at the definition; a dependent one only in an instantiation.
template <class T> int g (T t, S s1, S s2)
{
  S s3 = s1 + s2;
  return t + s3.v;
}
static_assert (parent_of (resolved_operator_of (nth_token (^^g, "+", 0))) == ^^S);
constexpr info dep = resolved_operator_of (nth_token (^^g, "+", 1));  // { dg-error "'the operator is template-dependent; its resolution is known only in an instantiation'" }

// A token from a literal is not a token of a recorded definition.
constexpr info lit = resolved_operator_of (^^{ + });  // { dg-error "'the resolution of the operator is not available; it is known only for the tokens of a recorded definition'" }
constexpr info emp = resolved_operator_of (^^{});  // { dg-error "'token sequence is empty'" }

// A definition that is not recorded.
S h (S a, S b) { return a + b; }  // { dg-error "'the tokens of the definition are not available" "" { target *-*-* } .+1 }
constexpr info nrec = resolved_operator_of (nth_token (^^h, "+", 0));
