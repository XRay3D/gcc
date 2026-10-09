// P3294 token injection: std::meta::queue_injection at namespace scope.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -ftoken-injection" }

#include <meta>
#include <string_view>

using namespace std::meta;
using sv = std::string_view;

consteval { queue_injection (^^{ int global_x = 42; }); }
int &global_ref = global_x;

consteval {
  queue_injection (^^{ constexpr int first = 1; });
  queue_injection (^^{ constexpr int second = first + 1; });
}
static_assert (first == 1 && second == 2);

// Several declarations in a single sequence.
consteval {
  queue_injection (^^{
    struct Injected { int member; };
    constexpr Injected injected{7};
    constexpr int get () { return injected.member; }
  });
}
static_assert (get () == 7);

// Nothing is injected before the consteval block is complete.
constexpr int before = 1;
consteval {
  queue_injection (^^{ constexpr int after = before + 1; });
  static_assert (before == 1);
}
static_assert (after == 2);

namespace ns {
consteval { queue_injection (^^{ constexpr int in_ns = 1; }); }
static_assert (in_ns == 1);
static_assert (nearest_namespace () == ^^ns);
static_assert (nearest_token_queuing_context () == ^^ns);
static_assert (nearest_class_or_namespace () == ^^ns);

namespace inner {
// An explicit context naming an enclosing namespace: the tokens are injected
// when the parser gets back to that namespace.
consteval { queue_injection (^^ns, ^^{ constexpr int from_inner = 2; }); }
consteval { queue_injection (^^::, ^^{ constexpr int from_inner_global = 3; }); }
constexpr int in_inner = 4;
}  // namespace inner
static_assert (from_inner == 2);
static_assert (inner::in_inner == 4);

// Queued at the very end of the namespace.
consteval { queue_injection (^^{ constexpr int last_in_ns = 5; }); }
}  // namespace ns
static_assert (ns::in_ns == 1);
static_assert (ns::from_inner == 2);
static_assert (ns::last_in_ns == 5);
static_assert (from_inner_global == 3);
static_assert (nearest_namespace () == ^^::);

namespace interpolated_code {
consteval void make_powers (int count)
{
  for (int i = 0; i < count; ++i)
    queue_injection (^^{ constexpr long \["pow2_", i] = \val(1L << i); });
}
consteval { make_powers (5); }
static_assert (pow2_0 == 1 && pow2_1 == 2 && pow2_4 == 16);
static_assert (__is_same (decltype (pow2_3), const long));

consteval info make_sum (sv name, int n)
{
  list_builder params;
  list_builder terms (^^{ + });
  for (int i = 0; i < n; ++i)
    {
      params += ^^{ long \["a", i] };
      terms += ^^{ \["a", i] };
    }
  return ^^{ constexpr long \[name](\{params}) { return \{terms}; } };
}
consteval { queue_injection (make_sum ("sum4", 4)); }
static_assert (sum4 (1, 2, 3, 4) == 10);

consteval void make_enum (sv name, const char *const *values, int n)
{
  list_builder items;
  for (int i = 0; i < n; ++i)
    items += ^^{ \[values[i]] };
  queue_injection (^^{ enum class \[name] { \{items} }; });
}
constexpr const char *fruits[] = {"apple", "banana", "cherry"};
consteval { make_enum ("Fruit", fruits, 3); }
static_assert (int (Fruit::apple) == 0 && int (Fruit::cherry) == 2);

// Splices and string literals.
consteval info declare (info type, sv name, int init)
{
  return ^^{ constexpr \[:type:] \[name] = \val(init); };
}
consteval {
  queue_injection (declare (^^int, "spliced_int", 42));
  queue_injection (declare (^^long, "spliced_long", 43));
  queue_injection (^^{ constexpr const char *text = \str(sv("some \"text\"\n")); });
}
static_assert (spliced_int == 42 && __is_same (decltype (spliced_int), const int));
static_assert (spliced_long == 43 && __is_same (decltype (spliced_long), const long));
static_assert (text[0] == 's' && text[5] == '"' && text[11] == '\n' && text[12] == 0);

// Injected code can itself inject code.
consteval {
  queue_injection (^^{
    consteval { queue_injection (^^{ constexpr int nested = 1; }); }
    constexpr int uses_nested = nested + 1;
  });
}
static_assert (nested == 1 && uses_nested == 2);

// A token sequence whose interpolators are evaluated later.
consteval {
  queue_injection (^^{
    consteval { queue_injection (^^{ constexpr int \["late_", 7] = \val(7); }); }
  });
}
static_assert (late_7 == 7);

// Templates.
consteval {
  queue_injection (^^{
    template <typename T> constexpr T twice (T t) { return t + t; }
    template <typename T> struct Box { T value; };
  });
}
static_assert (twice (21) == 42 && Box<int>{3}.value == 3);

// A value of class type.
struct P { int a, b; };
consteval { queue_injection (^^{ constexpr P p = \val(P{3, 4}); }); }
static_assert (p.a == 3 && p.b == 4);
}  // namespace interpolated_code

namespace errors {
consteval { queue_injection (^^{ int x = undeclared; }); }  // { dg-message "in token sequence injected here" }
// { dg-error "'undeclared' was not declared in this scope" "" { target *-*-* } .-1 }

consteval { queue_injection (^^int, ^^{ int w; }); }  // { dg-error "cannot inject tokens into '\\^\\^int'" }

consteval { queue_injection (^^int); }  // { dg-error "'std::meta::queue_injection' called with a reflection that is not a token sequence" }

namespace closed {}
consteval { queue_injection (^^closed, ^^{ int v; }); }  // { dg-error "cannot inject tokens into '\\^\\^errors::closed'" }

// Injection requires a consteval block.
constexpr int not_plain = (queue_injection (^^{ int u; }), 0);  // { dg-message "in 'constexpr' expansion" }
// { dg-error "'std::meta::queue_injection' not evaluated from 'consteval' block" "" { target *-*-* } 0 }

// The tokens queued by a consteval block that is not a constant expression
// are not injected.
consteval {
  queue_injection (^^{ int discarded; });
  throw 1;
}  // { dg-error "uncaught exception" }
int discarded;
}  // namespace errors

// Queued at the very end of the translation unit.
consteval { queue_injection (^^{ static_assert (ns::in_ns == 1); int at_eof = 0; }); }
