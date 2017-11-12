#if !defined(STIPULATION_SLICE_INSERTION_H)
#define STIPULATION_SLICE_INSERTION_H

/* Functionality related to "branches"; i.e. (chains of) pipe slices
 */

#include "stipulation/structure_traversal.h"

/* Try insertion before a slice
 * @param st structure representing the insertion traversal
 */
boolean slice_insertion_insert_before(slice_index si,
                                      stip_structure_traversal *st);

enum
{
  no_slice_rank = INT_MAX
};

typedef enum
{
  branch_slice_rank_order_recursive,
  branch_slice_rank_order_nonrecursive
} branch_slice_rank_order_type;

typedef struct branch_slice_insertion_state_type
{
    slice_index const *prototypes;
    unsigned int nr_prototypes;
    slice_index const *slice_rank_order;
    unsigned int nr_slice_rank_order_elmts;
    unsigned int nr_exit_slice_types;
    branch_slice_rank_order_type type;
    unsigned int base_rank;
    slice_index prev;
    stip_structure_traversal *parent;
} branch_slice_insertion_state_type;

/* Determine the rank of a slice type
 * @param type slice type
 * @return rank of type; no_slice_rank if the rank can't be determined
 */
unsigned int get_slice_rank(slice_type type,
                            branch_slice_insertion_state_type const *state);

/* Initialise a structure traversal for inserting slices into a branch
 * @param st address of structure representing the traversal
 * @param state address of structure representing the insertion
 * @param context initial context of the insertion traversal; typically the
 *                current context of a surrounding traversal that has arrived
 *                at the slice where the insertion is to start
 */
void slice_insertion_init_traversal(stip_structure_traversal *st,
                                    branch_slice_insertion_state_type *state,
                                    stip_traversal_context_type context);

/* Deallocate an array of slice insertion prototypes
 * @param prototypes array of prototypes
 * @param nr_prototypes size of prototypes
 */
void deallocate_slice_insertion_prototypes(slice_index const prototypes[],
                                           unsigned int nr_prototypes);

/* Insert slices into a branch.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by slice_insertion_insert().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void slice_insertion_insert(slice_index si,
                            slice_index const prototypes[],
                            unsigned int nr_prototypes);

/* Type of pointers to contextual slice inserter functions
 */
typedef void (*slice_inserter_contextual_type)(slice_index si,
                                               stip_traversal_context_type context,
                                               slice_index const prototypes[],
                                               unsigned int nr_prototypes);

/* Insert slices into a branch, taking into account the context of a structure
 * traversal that led to the insertion point.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by leaf_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param context context of a traversal at slice is
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void slice_insertion_insert_contextually(slice_index si,
                                         stip_traversal_context_type context,
                                         slice_index const prototypes[],
                                         unsigned int nr_prototypes);

/* Prepare a structure traversal object for slice insertion according to a
 * factored out slice type order. After slice_insertion_prepare_factored_order()
 * has returned:
 * - further customise the traversal object according to the respective neeeds
 * - invoke stip_traverse_structure_children_pipe(si,st_nested)
 * @param si identifies starting point of insertion
 * @param st insertion traversal where we come from and will return to
 * @param st_nested traversal object to be prepared
 * @param state_nested to hold state of *st_nested; *state_nested must be
 *                     defined in the same scope as *st_nested
 * @param order factored slice type order
 * @param nr_order number of elements of order
 * @param end_of_factored_order slice type where to return to insertion defined
 *                              by st
 */
void slice_insertion_prepare_factored_order(slice_index si,
                                            stip_structure_traversal *st,
                                            stip_structure_traversal *st_nested,
                                            branch_slice_insertion_state_type *state_nested,
                                            slice_index const order[],
                                            unsigned int nr_order,
                                            unsigned int nr_exit_slice_types);

/* Visist a pipe slice during an insertion traversal
 * @param si identifies the pipe slice
 * @param st holds the current state of the insertion traversal
 * @note this function is typically called automatically by insertion traversal;
 *       user code make take its address to override behviour of an insertion
 *       traversal at a specific slice type after having initialised the
 *       traversal using slice_insertion_init_traversal()
 */
void insert_visit_pipe(slice_index si, stip_structure_traversal *st);

#endif
