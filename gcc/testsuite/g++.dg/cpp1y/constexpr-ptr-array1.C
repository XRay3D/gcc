// Pointers into arrays and references to their elements during constant
// evaluation: arithmetic, comparisons, and binding references.
// { dg-do compile { target c++14 } }

constexpr int g[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

constexpr int
sum_by_pointer ()
{
  int s = 0;
  for (const int *p = g, *e = g + 8; p != e; ++p)
    s += *p;
  return s;
}
static_assert (sum_by_pointer () == 36, "");

constexpr bool
compare ()
{
  const int *p = g + 2;
  const int *q = p + 3;		// g + 5
  const int *r = q - 3;		// back to g + 2
  const int *e = g + 8;		// one past the end
  return (p == r && p != q && p < q && q > p && p <= r && r >= p
	  && !(q < p) && e > q && e == q + 3 && q - p == 3 && *r == 3
	  && p + 0 == p && q - 5 == g && *(q - 5) == 1);
}
static_assert (compare (), "");

constexpr const int &
elem (const int *p, int i)
{
  return p[i];
}

constexpr int
by_reference ()
{
  const int &a = elem (g + 1, 2);	// g[3]
  const int &b = elem (g, 3);
  return (&a == &b) * 100 + a;
}
static_assert (by_reference () == 104, "");

constexpr void
set (int &r, int v)
{
  r = v;
}

constexpr int
write_through_reference ()
{
  int a[4] = { 0, 0, 0, 0 };
  int *p = a + 1;
  set (p[1], 7);			// a[2]
  set (*(p + 2), 9);			// a[3]
  int *q = p + 2;
  return a[2] * 10 + a[3] + (q - a) * 100 + (q > p);
}
static_assert (write_through_reference () == 380, "");

constexpr char s[] = "hello";

constexpr int
find (const char *b, const char *e, char c)
{
  for (const char *p = b; p < e; p = p + 1)
    if (*p == c)
      return p - b;
  return -1;
}
static_assert (find (s, s + 5, 'l') == 2, "");
static_assert (find (s + 3, s + 5, 'o') == 1, "");
static_assert (find (s, s + 5, 'x') == -1, "");

constexpr int bad = *(g + 8);	// { dg-error "outside the bounds|array subscript" }
