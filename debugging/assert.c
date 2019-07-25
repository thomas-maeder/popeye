#include "debugging/assert.h"
#include "options/movenumbers.h"
#include "solving/move_generator.h"
#include "pieces/attributes/total_invisible.h"

#include <assert.h>

void assert_impl(char const *assertion, const char *file, int line)
{
  move_generator_write_history();
  total_invisible_write_flesh_out_history();
  move_numbers_write_history();

#ifndef EMSCRIPTEN

#if defined(_WIN32) || defined(_WIN64)
  /* why can't these guys do anything in a standard conforming way??? */
  _assert
#else
  __assert
#endif
  (assertion,file,line);

#endif
}
