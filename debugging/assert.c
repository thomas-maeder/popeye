#include "debugging/assert.h"
#include "options/movenumbers.h"
#include "solving/move_generator.h"

void assert_impl(char const *assertion, const char *file, int line)
{
  move_generator_write_history();
  move_numbers_write_history();
  __assert(assertion,file,line);
}
