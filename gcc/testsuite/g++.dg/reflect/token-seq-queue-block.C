// P3294 token injection: std::meta::queue_injection at block scope.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>

using namespace std::meta;

namespace block_scope {
constexpr int f ()
{
  static_assert (nearest_token_queuing_context () == ^^f);
  static_assert (nearest_class_or_namespace () == ^^block_scope);
  static_assert (nearest_namespace () == ^^block_scope);

  consteval { queue_injection (^^{ int x = 40; }); }
  consteval { queue_injection (^^{ x += 1; ++x; }); }
  return x;
}
static_assert (f () == 42);

// Statements of all kinds.
constexpr int g (int n)
{
  int result = 0;
  consteval {
    queue_injection (^^{
      for (int i = 0; i < n; ++i)
	result += i;
      if (result > 100) { return -1; }
      struct Local { int v = 1000; };
      result += Local ().v;
    });
  }
  return result;
}
static_assert (g (4) == 1006);
static_assert (g (100) == -1);

// Nested blocks: the tokens go to the innermost block.
constexpr int nested ()
{
  int x = 1;
  {
    consteval { queue_injection (^^{ int x = 2; }); }
    if (x != 2)
      return -1;
  }
  return x;
}
static_assert (nested () == 1);

// Lambdas and member functions.
constexpr auto lambda = [] {
  consteval { queue_injection (^^{ return 7; }); }
};
static_assert (lambda () == 7);

struct S {
  constexpr int m () const
  {
    consteval { queue_injection (^^{ return v * 2; }); }
  }
  int v = 21;
};
static_assert (S ().m () == 42);

// Generated statements using interpolators.
consteval void assign_all (const char *const *names, int n, int value)
{
  for (int i = 0; i < n; ++i)
    queue_injection (^^{ \[names[i]] = \val(value + i); });
}
constexpr int assigned ()
{
  int a = 0, b = 0, c = 0;
  static constexpr const char *names[] = {"a", "b", "c"};
  consteval { assign_all (names, 3, 10); }
  return a * 10000 + b * 100 + c;
}
static_assert (assigned () == 101112);

// At the very end of the block.
constexpr int at_end ()
{
  consteval { queue_injection (^^{ return 3; }); }
}
static_assert (at_end () == 3);

// A namespace-scope declaration requested from a block.
void from_block ()
{
  consteval { queue_injection (^^block_scope, ^^{ constexpr int from_function = 5; }); }
}
static_assert (from_function == 5);

// A consteval block in a statement that could also be an expression.
constexpr int in_if ()
{
  if (true)
    {
      consteval { queue_injection (^^{ return 11; }); }
    }
  return 0;
}
static_assert (in_if () == 11);
}  // namespace block_scope

namespace errors {
void other ();

void f ()
{
  consteval { queue_injection (^^{ undeclared (); }); }  // { dg-error "'undeclared' was not declared in this scope" }
  // { dg-message "in token sequence injected here" "" { target *-*-* } .-1 }

  consteval { queue_injection (^^other, ^^{ int x; }); }  // { dg-error "cannot inject tokens into" }

}

// Injection from a template instantiation is not supported yet.
template <typename T>
void tmpl ()
{
  consteval { queue_injection (^^{ T t; }); }  // { dg-message "sorry, unimplemented: token injection from a 'consteval' block in a template instantiation" }
}
template void tmpl<int> ();
}  // namespace errors
