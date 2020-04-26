#if !defined(POSITION_SIDE_ENUM_H)
#define POSITION_SIDE_ENUM_H

typedef enum
{
 White, Black, nr_sides, no_side = nr_sides
} Side;
extern char const *Side_names[];
/* include side.enum to make sure that all the dependencies are generated correctly: */
#include "side.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
