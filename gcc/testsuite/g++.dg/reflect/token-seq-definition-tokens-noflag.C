// -fdefinition-tokens needs -ftoken-injection.
// { dg-do compile { target c++26 } }
// { dg-additional-options "-freflection -fdefinition-tokens" }
// { dg-error "'-fdefinition-tokens' requires '-ftoken-injection'" "" { target *-*-* } 0 }

int x;
