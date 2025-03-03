#include "debugging/assert.h"
#include "options/movenumbers.h"
#include "solving/move_generator.h"
#include "pieces/attributes/total_invisible.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__GNUC__)

void assert_impl(char const *assertion, char const *file, int line, char const *func)
{
  static boolean recursion_guard = false;

  if (!recursion_guard)
  {
    recursion_guard = true;

    move_generator_write_history();
    total_invisible_write_flesh_out_history();
    move_numbers_write_history();

    recursion_guard = false;
  }

  fprintf(stderr,"%s:%d: %s: Assertion `%s' failed.\n",file,line,func,assertion);
  exit(1);
}

#endif
