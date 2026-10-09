// A checked std::vector built from its own tokens: a consteval block walks
// the members of std::vector<int> and injects, for operator[], at, front,
// back, push_back and pop_back, a function with the same signature whose
// body is the libstdc++ body (from definition_tokens_of) with a log line and
// a bounds check inserted after the opening brace.  Adapted from EDG's
// tests/tests/reflections/tokens_d.sft.cpp.  The tokens of the members of
// class templates are always recorded, so -fdefinition-tokens is not needed.
// { dg-do run { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace std::meta;

void trace (const char *msg) { std::printf ("[trace] %s\n", msg); }
void trace (const char *msg, std::size_t n)
{ std::printf ("[trace] %s, index %zu\n", msg, n); }

namespace patch {
using base = std::vector<int>;

consteval bool wanted (info m)
{
  if (!is_function (m) || is_special_member_function (m)) return false;
  if (is_operator_function (m))
    return operator_of (m) == operators::op_square_brackets;
  if (!has_identifier (m)) return false;
  std::string_view n = identifier_of (m);
  return n == "at" || n == "front" || n == "back" || n == "push_back"
	 || n == "pop_back";
}

consteval std::string name_of (info m)
{
  std::string name = "operator[]";
  if (!is_operator_function (m)) name = identifier_of (m);
  return name;
}

consteval bool has_index (info m)
{
  for (info p : parameters_of (m))
    if (has_identifier (p) && identifier_of (p) == "__n") return true;
  return false;
}

consteval info declarator (info m)
{
  list_builder params;
  int unnamed = 0;
  for (info p : parameters_of (m))
    if (has_identifier (p))
      params += ^^{ typename \[: type_of (p) :] \[identifier_of (p)] };
    else
      params += ^^{ typename \[: type_of (p) :] \["__unnamed", unnamed++] };
  info name = is_operator_function (m) ? ^^{ operator[] }
				       : ^^{ \[identifier_of (m)] };
  info d = ^^{ typename \[: return_type_of (m) :] \{name} ( \{params} ) };
  if (is_const (m)) d = ^^{ \{d} const };
  return d;
}

consteval info body (info m)
{
  std::vector<info> t = tokens_of (definition_tokens_of (m));
  std::string name = name_of (m);
  std::string full = "checked_vector::" + name;
  if (is_const (m)) full += " const";
  std::string_view what = std::define_static_string (full);
  info out = t.front ();
  if (has_index (m))
    out = ^^{ \{out} trace (\str(what), __n); };
  else
    out = ^^{ \{out} trace (\str(what)); };
  if (name == "operator[]")
    {
      std::string_view msg = std::define_static_string (
	"checked_vector::operator[]: index out of range");
      out = ^^{ \{out}
	if (__n >= this->size ()) throw std::out_of_range (\str(msg)); };
    }
  else if (name == "front" || name == "back" || name == "pop_back")
    {
      std::string_view msg
	= std::define_static_string ("checked_vector::" + name + ": empty vector");
      out = ^^{ \{out} if (this->empty ()) throw std::out_of_range (\str(msg)); };
    }
  for (std::size_t k = 1; k < t.size (); ++k) out = ^^{ \{out} \{t[k]} };
  return out;
}
}

struct checked_vector : std::vector<int> {
  using std::vector<int>::vector;
  using _Alloc_traits = __gnu_cxx::__alloc_traits<std::allocator<int>>;
  consteval {
    for (info m : members_of (^^patch::base, access_context::unchecked ()))
      if (patch::wanted (m))
	queue_injection (^^{ \{patch::declarator (m)} \{patch::body (m)} });
  }
};

int main ()
{
  checked_vector v{1, 2, 3};
  v.push_back (4);
  int v1 = v[1];
  int at2 = v.at (2);
  int front = v.front ();
  int back = v.back ();
  std::printf ("v[1] = %d, at(2) = %d, front = %d, back = %d\n", v1, at2,
	       front, back);
  v.pop_back ();
  try { std::printf ("v[10] = %d\n", v[10]); }
  catch (const std::out_of_range &e) { std::printf ("caught: %s\n", e.what ()); }
  checked_vector e;
  try { e.pop_back (); }
  catch (const std::out_of_range &x) { std::printf ("caught: %s\n", x.what ()); }
}

// { dg-output "\\\[trace\\\] checked_vector::push_back\n" }
// { dg-output "\\\[trace\\\] checked_vector::operator\\\[\\\], index 1\n" }
// { dg-output "\\\[trace\\\] checked_vector::at, index 2\n" }
// { dg-output "\\\[trace\\\] checked_vector::operator\\\[\\\], index 2\n" }
// { dg-output "\\\[trace\\\] checked_vector::front\n" }
// { dg-output "\\\[trace\\\] checked_vector::back\n" }
// { dg-output "v\\\[1\\\] = 2, at\\(2\\) = 3, front = 1, back = 4\n" }
// { dg-output "\\\[trace\\\] checked_vector::pop_back\n" }
// { dg-output "\\\[trace\\\] checked_vector::operator\\\[\\\], index 10\n" }
// { dg-output "caught: checked_vector::operator\\\[\\\]: index out of range\n" }
// { dg-output "\\\[trace\\\] checked_vector::pop_back\n" }
// { dg-output "caught: checked_vector::pop_back: empty vector\n" }
