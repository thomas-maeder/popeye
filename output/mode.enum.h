typedef enum
{
 output_mode_tree, output_mode_line, output_mode_none
} output_mode;
extern char const *output_mode_names[];
/* include mode.enum to make sure that all the dependencies are generated correctly: */
#include "mode.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS
