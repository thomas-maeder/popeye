typedef enum
{
 colour_white, colour_black, colour_neutral, nr_colours
} Colour;
extern char const *Colour_names[];
/* include color.enum to make sure that all the dependencies are generated correctly: */
#include "color.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS
