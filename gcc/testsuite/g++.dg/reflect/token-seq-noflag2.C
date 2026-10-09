// -ftoken-injection needs -freflection.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-ftoken-injection" }
// { dg-error "'-ftoken-injection' requires '-freflection'" "" { target *-*-* } 0 }

int x;
