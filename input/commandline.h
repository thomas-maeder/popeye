#if !defined(INPUT_COMMANDLINE_H)
#define INPUT_COMMANDLINE_H

#include "stipulation/stipulation.h"

void command_line_options_parser_solve(slice_index si);

slice_index alloc_command_line_options_parser(int argc, char **argv);

#endif
