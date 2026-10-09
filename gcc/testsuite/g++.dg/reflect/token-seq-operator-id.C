// An operator-function-id is one element of std::meta::tokens_of, of kind
// token_kind::operator_function_id, and std::meta::operator_of accepts a
// token sequence that is one (an extension to P3294, as in EDG and
// clang-p2996).
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

constexpr info ops = ^^{ a.operator+(b); v.operator[](1);
  operator new[](8); c += d; operator bitor; operator int; operator ""_x;
  operator ( ) operator delete [ ] operator co_await operator <=> operator ->*
  operator };

static_assert (tokens_of (ops).size () == 37);
static_assert (token_spelling_of (tokens_of (ops)[35]) == "operator ->*");
static_assert (token_kind_of (tokens_of (ops)[36]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (ops)[2]) == token_kind::operator_function_id);
static_assert (token_spelling_of (tokens_of (ops)[2]) == "operator +");
static_assert (token_kind_of (tokens_of (ops)[9]) == token_kind::operator_function_id);
static_assert (token_spelling_of (tokens_of (ops)[9]) == "operator [ ]");
static_assert (token_spelling_of (tokens_of (ops)[14]) == "operator new [ ]");
static_assert (token_kind_of (tokens_of (ops)[19]) == token_kind::identifier);
static_assert (token_kind_of (tokens_of (ops)[20]) == token_kind::punctuator);
static_assert (token_spelling_of (tokens_of (ops)[23]) == "operator bitor");
// operator int and operator ""_x are not grouped.
static_assert (token_kind_of (tokens_of (ops)[25]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (ops)[26]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (ops)[28]) == token_kind::keyword);
static_assert (token_kind_of (tokens_of (ops)[29]) == token_kind::user_defined_literal);
static_assert (token_spelling_of (tokens_of (ops)[31]) == "operator ( )");
static_assert (token_spelling_of (tokens_of (ops)[32]) == "operator delete [ ]");
static_assert (token_spelling_of (tokens_of (ops)[33]) == "operator co_await");
static_assert (token_spelling_of (tokens_of (ops)[34]) == "operator <=>");
// A trailing operator is not followed by an operator.
constexpr info last = ^^{ operator ->* operator };
static_assert (tokens_of (last).size () == 2);
static_assert (token_kind_of (tokens_of (last)[1]) == token_kind::keyword);

static_assert (token_kind_of (^^{ operator + }) == token_kind::operator_function_id);
static_assert (tokens_of (^^{ operator new [ ] x }).size () == 2);
static_assert (token_kind_of (^^{ operator int }) == token_kind::keyword);

// operator_of gives the std::meta::operators enumerator.
static_assert (operator_of (^^{ operator + }) == operators::op_plus);
static_assert (operator_of (^^{ operator ( ) }) == operators::op_parentheses);
static_assert (operator_of (^^{ operator [ ] }) == operators::op_square_brackets);
static_assert (operator_of (^^{ operator new }) == operators::op_new);
static_assert (operator_of (^^{ operator new [ ] }) == operators::op_array_new);
static_assert (operator_of (^^{ operator delete [ ] }) == operators::op_array_delete);
static_assert (operator_of (^^{ operator co_await }) == operators::op_co_await);
static_assert (operator_of (^^{ operator bitor }) == operators::op_pipe);
static_assert (operator_of (^^{ operator = }) == operators::op_equals);
static_assert (operator_of (^^{ operator += }) == operators::op_plus_equals);
static_assert (operator_of (^^{ operator ++ }) == operators::op_plus_plus);
static_assert (operator_of (^^{ operator -> }) == operators::op_arrow);
static_assert (operator_of (^^{ operator ->* }) == operators::op_arrow_star);
static_assert (operator_of (^^{ operator <=> }) == operators::op_spaceship);
static_assert (operator_of (^^{ operator , }) == operators::op_comma);
static_assert (symbol_of (operator_of (^^{ operator >>= })) == ">>=");
struct S { S operator+ (S) const; };
static_assert (operator_of (^^S::operator+) == operators::op_plus);

// A bare operator token is not a name, but "operator" can be put in front of
// it to classify it.
consteval bool is_operator_token (info t)
{
  return token_kind_of (^^{ operator \{t} }) == token_kind::operator_function_id;
}
consteval operators operator_of_token (info t)
{
  return operator_of (^^{ operator \{t} });
}
static_assert (is_operator_token (tokens_of (^^{ c += d })[1]));
static_assert (operator_of_token (tokens_of (^^{ c += d })[1]) == operators::op_plus_equals);
static_assert (!is_operator_token (tokens_of (^^{ c += d })[0]));
static_assert (!is_operator_token (tokens_of (^^{ a :: b })[1]));

// An operator-function-id has no value, and is not a function.
constexpr info v1 = token_value_of (^^{ operator + });  // { dg-error "'the token is an operator-function-id, which has no value'" }
static_assert (!is_operator_function (^^{ operator + }));

constexpr operators o1 = operator_of (^^{ + });  // { dg-error "'token sequence is not exactly one operator-function-id'" }
constexpr operators o2 = operator_of (^^{ operator + x });  // { dg-error "'token sequence is not exactly one operator-function-id'" }
constexpr operators o3 = operator_of (^^{ operator int });  // { dg-error "'token sequence is not exactly one operator-function-id'" }
