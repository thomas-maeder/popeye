/* This is dhtsimple.c --  Version 1.5
 * This code is copyright by
 *  Elmar Bartel 1993-99
 *  Institut fuer Informatik, TU Muenchen, Germany  
 *  bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept intact
 * and in place.
 */
#include "debugging/assert.h"
#include "dhtvalue.h"
#include <stdint.h>

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
static unsigned long ConvertSimpleValue(dhtKey k)
{
#  if (defined(__cplusplus) && (__cplusplus >= 201103L)) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  unsigned long long a, b, c;
  c = 0x9e3779b97f4a7c13LLU;
#  else
#    if !((((-1LU) >> 31) >> 31) >> 1)
#        error "ERROR: unable to build dhtsimpl.c due to lack of a sufficiently-large integer type."
#    endif
  unsigned long a, b, c;
  c = 0x9e3779b97f4a7c13LU;
#  endif
  a = k.value.unsigned_integer<<1;
  b = k.value.unsigned_integer;
  mix(a,b,c);
  return (unsigned long)c;
}
#else
static unsigned long ConvertSimpleValue(dhtKey k)
{
  unsigned long c = k.value.unsigned_integer;
  unsigned long a = 0;
  unsigned long b = 0x9e3779b9U;
  a -= c;
  a ^= c >> 13;
  b -= c;  b -= a;  b ^= (a << 8); 
  c -= a;  c -= b;  c ^= (b >> 13);
  a -= b;  a -= c;  a ^= (c >> 12);
  b -= c;  b -= a;  b ^= (a << 16);
  c -= a;  c -= b;  c ^= (b >> 5);
  a -= b;  a -= c;  a ^= (c >> 3); 
  b -= c;  b -= a;  b ^= (a << 10);
  c -= a;  c -= b;  c ^= (b >> 15);
  return c;
}
#endif /*ARCH64*/

static int EqualSimpleValue(dhtKey k1, dhtKey k2)
{
  return (k1.value.unsigned_integer == k2.value.unsigned_integer);
}

static int DupSimpleValue(dhtValue kv, dhtValue *output)
{
  assert(!!output);
  *output = kv;
  return 0;
}

static void FreeSimpleValue(dhtValue kv)
{
  (void)kv;
}

static void DumpSimpleValue(dhtValue kv, FILE *f)
{
  assert(!!f);
#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
    (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
  fprintf(f, "%08jx", (uintmax_t)kv.unsigned_integer);
#else
  fprintf(f, "%08lx", (unsigned long int)kv.unsigned_integer);
#endif
}

dhtValueProcedures dhtSimpleProcs =
{
  ConvertSimpleValue,
  EqualSimpleValue,
  DupSimpleValue,
  FreeSimpleValue,
  DumpSimpleValue
};
