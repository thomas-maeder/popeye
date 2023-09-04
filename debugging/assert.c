#include "debugging/assert.h"
#include "options/movenumbers.h"
#include "solving/move_generator.h"
#include "pieces/attributes/total_invisible.h"

#include <assert.h>

void assert_impl(char const *assertion, const char *file, int line)
{
  static boolean recursion_guard = false;

  if (!recursion_guard)
  {
    recursion_guard = true;

    move_generator_write_history();
    total_invisible_write_flesh_out_history();
    move_numbers_write_history(nbply);

    recursion_guard = false;
  }

#ifndef EMSCRIPTEN

#if !defined(NDEBUG)

#ifdef __CYGWIN__
  __assert(assertion,line,file);
#else
#    if defined(_WIN32) || defined(_WIN64)
  /* why can't these guys do anything in a standard conforming way??? */
  _assert
#    else
  __assert
#    endif
  (assertion,file,line);
#endif

#endif

#endif
}
