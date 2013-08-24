#if !defined(STIPULATION_MOVES_TRAVERSAL_H)
#define STIPULATION_MOVES_TRAVERSAL_H

#include "stipulation/slice.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/goals/goals.h"
#include "output/mode.h"

#include <stddef.h>

struct stip_moves_traversal;

/* Type of visitors for stipulation traversals
 */
typedef void (*stip_moves_visitor)(slice_index si,
                                   struct stip_moves_traversal *st);

/* Map a slice type to a visitor */
typedef struct
{
    stip_moves_visitor visitors[nr_slice_types];
} moves_visitor_map_type;

/* hold the state of a moves traversal */
typedef struct stip_moves_traversal
{
    moves_visitor_map_type map;
    unsigned int remaining_watermark[max_nr_slices];
    stip_traversal_context_type context;
    stip_traversal_activity_type activity;
    stip_length_type full_length;
    stip_length_type remaining;
    void *param;
} stip_moves_traversal;

enum
{
  STIP_MOVES_TRAVERSAL_LENGTH_UNINITIALISED = INT_MAX
};

/* define an alternative visitor for a particular slice type */
typedef struct
{
    slice_type type;
    stip_moves_visitor visitor;
} moves_traversers_visitors;

/* Initialise a move traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_moves_traversal_init(stip_moves_traversal *st, void *param);

/* Set the number of moves at the start of the traversal. Normally, this is
 * determined while traversing the stipulation. Only invoke
 * stip_moves_traversal_set_remaining() when the traversal is started in the
 * middle of a stipulation.
 * @param st to be initialised
 * @param remaining number of remaining moves (without slack)
 * @param full_length full number of moves of the initial branch (without slack)
 */
void stip_moves_traversal_set_remaining(stip_moves_traversal *st,
                                        stip_length_type remaining,
                                        stip_length_type full_length);

/* Override the behavior of a moves traversal at some slice types
 * @param st to be initialised
 * @param moves_traversers_visitors array of alternative visitors; for
 *                                  slices with types not mentioned in
 *                                  moves_traversers_visitors, the default
 *                                  visitor will be used
 * @param nr_visitors length of moves_traversers_visitors
 */
void stip_moves_traversal_override(stip_moves_traversal *st,
                                   moves_traversers_visitors const visitors[],
                                   unsigned int nr_visitors);

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_structure(stip_moves_traversal *st,
                                                slice_structural_type type,
                                                stip_moves_visitor visitor);

/* Override the behavior of a moves traversal at slices of a functional type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_by_function(stip_moves_traversal *st,
                                               slice_functional_type,
                                               stip_moves_visitor visitor);

/* Override the behavior of a moves traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_moves_traversal_override_single(stip_moves_traversal *st,
                                          slice_type type,
                                          stip_moves_visitor visitor);

/* Traversal of moves of the stipulation
 * @param root identifies start of the stipulation (sub)tree
 * @param st address of data structure holding parameters for the operation
 */
void stip_traverse_moves(slice_index root, stip_moves_traversal *st);

/* Traversal of the moves beyond some root slice
 * @param si identifies root of subtree
 * @param st address of structure representing traversal
 */
void stip_traverse_moves_root(slice_index si, stip_moves_traversal *st);

/* Traverse operand 1 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_binary_operand1(slice_index binary_slice,
                                         stip_moves_traversal *st);

/* Traverse operand 2 of a binary slice
 * @param binary_slice identifies the binary slice
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_binary_operand2(slice_index binary_slice,
                                         stip_moves_traversal *st);

/* (Approximately) depth-first traversl of a stipulation sub-tree
 * @param root root of the sub-tree to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_moves_children(slice_index si,
                                  stip_moves_traversal *st);

/* Initialise stipulation traversal properties at start of program */
void init_moves_children_visitors(void);

#endif
