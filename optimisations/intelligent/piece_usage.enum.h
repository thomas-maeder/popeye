#if !defined(OPTIMISATIONS_INTELLIGENT_PIECE_USAGE_ENUM_H)
#define OPTIMISATIONS_INTELLIGENT_PIECE_USAGE_ENUM_H

typedef enum
{
 piece_is_unused, piece_pins, piece_is_fixed_to_diagram_square, piece_intercepts, piece_intercepts_check_from_guard, piece_blocks, piece_guards, piece_gives_check, piece_is_missing, piece_is_captured, piece_is_king
} piece_usage;
extern char const *piece_usage_names[];
/* include piece_usage.enum to make sure that all the dependencies are generated correctly: */
#include "piece_usage.enum"
#undef ENUMERATION_TYPENAME
#undef ENUMERATORS

#endif
