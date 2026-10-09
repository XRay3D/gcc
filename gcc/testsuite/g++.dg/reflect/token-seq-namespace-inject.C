// P3294 token injection: std::meta::namespace_inject.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <string_view>

using namespace std::meta;
using sv = std::string_view;

namespace paper_example {
consteval auto f (info r, int val, sv name)
{
  return ^^{ constexpr \[:r:] \[name] = \val(val); };
}

namespace N {}

consteval {
  queue_injection (^^{ static_assert (N::x == 42); });
  namespace_inject (^^N, f (^^int, 42, "x"));
}
static_assert (N::x == 42);

// The injection is immediate: the namespace is complete once the function
// returns.
consteval void inject_then_check ()
{
  namespace_inject (^^N, ^^{ constexpr int y = x + 1; });
  namespace_inject (^^N, ^^{ static_assert (y == 43); });
}
consteval { inject_then_check (); }
static_assert (N::y == 43);
}  // namespace paper_example

consteval { namespace_inject (^^{ constexpr int global_injected = 1; }); }
static_assert (global_injected == 1);

namespace open {
consteval {
  namespace_inject (^^{ constexpr int a = 1; });
  namespace_inject (^^open, ^^{ constexpr int b = a + 1; });
  namespace_inject (^^::, ^^{ constexpr int from_open = 3; });
}
static_assert (a == 1 && b == 2 && from_open == 3);

struct S {
  consteval { namespace_inject (^^{ constexpr int from_class = 4; }); }
  static_assert (from_class == 4);
  // The class member of the same name is not hidden by the injection.
  static constexpr int a = 100;
  static_assert (a == 100);
};

constexpr int f ()
{
  int from_function = -1;
  consteval {
    namespace_inject (^^{ constexpr int from_function = 5; constexpr int a_copy = a; });
  }
  // The local variable still hides the injected one.
  return from_function;
}
static_assert (f () == -1 && from_function == 5 && a_copy == 1);
}  // namespace open
static_assert (open::from_class == 4 && from_open == 3);

namespace closed {
namespace inner { constexpr int existing = 1; }
inline namespace inl { constexpr int in_inline = 2; }
namespace { constexpr int in_anon = 3; }
}  // namespace closed

consteval {
  namespace_inject (^^closed, ^^{ constexpr int a = 10; });
  namespace_inject (^^closed::inner, ^^{ constexpr int b = existing + 10; });
  namespace_inject (^^closed::inl, ^^{ constexpr int c = in_inline + 10; });
  namespace_inject (^^closed, ^^{
    namespace inner { constexpr int d = b + 1; }
    struct T { int t = 4; };
    template <typename U> constexpr U id (U u) { return u; }
  });
}
static_assert (closed::a == 10 && closed::inner::b == 11 && closed::c == 12);
static_assert (closed::inl::c == 12 && closed::inner::d == 12);
static_assert (closed::id (closed::T ().t) == 4);
static_assert (^^closed::a != ^^closed::inner::b);

namespace alias = closed::inner;
consteval { namespace_inject (^^alias, ^^{ constexpr int via_alias = 6; }); }
static_assert (closed::inner::via_alias == 6);

// Injected code that injects.
consteval {
  namespace_inject (^^closed, ^^{
    consteval { queue_injection (^^{ constexpr int queued = 7; }); }
    consteval { namespace_inject (^^closed::inner, ^^{ constexpr int deep = 8; }); }
    constexpr int after = queued + inner::deep;
  });
}
static_assert (closed::after == 15);

// Member functions of an injected class are parsed after the class.
consteval {
  namespace_inject (^^closed, ^^{
    struct M {
      constexpr int get () const { return v; }
      int v = 9;
    };
  });
}
static_assert (closed::M ().get () == 9);

namespace errors {
struct S {};

consteval { namespace_inject (^^S, ^^{ int x; }); }  // { dg-error "cannot inject tokens into '\\^\\^errors::S', which is not a namespace" }

consteval { namespace_inject (^^errors, ^^int); }  // { dg-error "'std::meta::namespace_inject' called with a reflection that is not a token sequence" }

consteval { namespace_inject (^^{ int y = undeclared; }); }  // { dg-error "'undeclared' was not declared in this scope" }
// { dg-message "in token sequence injected here" "" { target *-*-* } .-1 }

constexpr int not_plain = (namespace_inject (^^{ int u; }), 0);  // { dg-message "in 'constexpr' expansion" }
// { dg-error "'std::meta::namespace_inject' not evaluated from 'consteval' block" "" { target *-*-* } 0 }
}  // namespace errors
