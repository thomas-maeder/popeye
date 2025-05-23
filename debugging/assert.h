/* This module provides an assert() replacement that prints some contextual
 * informations before terminating because of a failed assert().
 */

/* There is intentionally no #include guard here. */

#if defined(NDEBUG)

#include <assert.h>

#elif !defined(__GNUC__) && !defined(__clang__)

#include <assert.h>

#elif defined(AUXILIARY)

/* no use to write auxiliary information on a failed assert() in auxiliary
 * programs */

#include <assert.h>

#else

#if !defined(ASSERT_IMPL_DECLARED)
void assert_impl(char const *assertion, char const *file, int line, char const *func);
#  define ASSERT_IMPL_DECLARED
#endif

#define assert(expr) ((expr) ? (void)0 : assert_impl(#expr,__FILE__,__LINE__,__func__))

#endif

/* Use
       STATIC_ASSERT(cond, msg);
   to test cond at compile-time (which must be possible).  If cond is false
   then compilation will fail.  This allows us to verify that our expected
   invariants still hold.  msg should be a string literal indicating the test
   and/or failure, to aid in correcting any errors identified.
   NOTE: The fallback option below has some limitations that, for portability,
   we should respect.
*/
#if !defined(STATIC_ASSERT)
#  if (defined(__cplusplus) && (__cplusplus >= 201103L)) || \
      (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L))
  #define STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#  elif (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L))
  #define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#  else
  /* This implementation is based on the May 2016 version at
         https://www.pixelbeat.org/programming/gcc/static_assert.html
     and we settle for the simple, universal version which cannot be used multiple
     times on the same line (or on the first line of a switch block).
     This implementation declares a useless enum.  Placing these inside a block
     will prevent those enums from leaking into the surrounding context.  Alas,
     that doesn't work at global scope.
  */       
  #define STATIC_ASSERT_CAT_IMPL_IMPL(a, b) a##b
  #define STATIC_ASSERT_CAT_IMPL(a, b) STATIC_ASSERT_CAT_IMPL_IMPL(a, b)
  #define STATIC_ASSERT(cond, msg) enum { \
                                     STATIC_ASSERT_CAT_IMPL(static_assert_failure_on_line_, __LINE__) = 1/(int)!!(cond) \
                                   }
#  endif
#endif
