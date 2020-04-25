/* This is dhtsimple.c --  Version 1.5
 * This code is copyright by
 *  Elmar Bartel 1993-99
 *  Institut fuer Informatik, TU Muenchen, Germany  
 *  bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept intact
 * and in place.
 */
#include "dhtvalue.h"

#if defined(ARCH64)
/* This code is derived from Bob Jenkins' work.
 * The original site is:
 *   http://ourworld.compuserve.com/homepages/bob_jenkins/blockcip.htm
 *                      ElB, 1999-09-16.
 */
#define mix(a,b,c)                              \
  {                                             \
    a -= b; a -= c; a ^= (c>>43);               \
    b -= c; b -= a; b ^= (a<<9);                \
    c -= a; c -= b; c ^= (b>>8);                \
    a -= b; a -= c; a ^= (c>>38);               \
    b -= c; b -= a; b ^= (a<<23);               \
    c -= a; c -= b; c ^= (b>>5);                \
    a -= b; a -= c; a ^= (c>>35);               \
    b -= c; b -= a; b ^= (a<<49);               \
    c -= a; c -= b; c ^= (b>>11);               \
    a -= b; a -= c; a ^= (c>>12);               \
    b -= c; b -= a; b ^= (a<<18);               \
    c -= a; c -= b; c ^= (b>>22);               \
  }
static unsigned long ConvertSimpleValue(dhtConstValue v)
{
  unsigned long a, b, c;
  c = 0x9e3779b97f4a7c13LL;
  a = v<<1;
  b = v;
  mix(a,b,c);
  return c;
}
#else
static unsigned long ConvertSimpleValue(dhtConstValue v)
{
  size_t c = (size_t)v; 
  size_t a = 0;
  size_t b = 0x9e3779b9;
  a -= c;
  a ^= c >> 13;
  b -= c;  b -= a;  b ^= a << 8; 
  c -= a;  c -= b;  c ^= b >> 13;
  a -= b;  a -= c;  a ^= c >> 12;
  b -= c;  b -= a;  b ^= a << 16;
  c -= a;  c -= b;  c ^= b >> 5; 
  a -= b;  a -= c;  a ^= c >> 3; 
  b -= c;  b -= a;  b ^= a << 10;
  c -= a;  c -= b;  c ^= b >> 15;
  return (unsigned long)c;
}
#endif /*ARCH64*/

static int EqualSimpleValue(dhtConstValue v1, dhtConstValue v2)
{
  return v1 == v2;
}

static dhtConstValue DupSimpleValue(dhtConstValue v)
{
  return v;
}

static void FreeSimpleValue(dhtValue v)
{
  return;
}

static void DumpSimpleValue(dhtConstValue v, FILE *f)
{
  fprintf(f, "%08lx", (unsigned long)(size_t)v);
}

dhtValueProcedures dhtSimpleProcs =
{
  ConvertSimpleValue,
  EqualSimpleValue,
  DupSimpleValue,
  FreeSimpleValue,
  DumpSimpleValue
};
