#if !defined(STIPULATION_TRAVERSAL_H)
#define STIPULATION_TRAVERSAL_H

/* Some types used for all stipulation traversals
 */

/* Solving context of the slice currently visited
 */
typedef enum
{
  stip_traversal_context_intro,   /* entering a branch */
  stip_traversal_context_attack,  /* approaching attack move in battle branch */
  stip_traversal_context_defense, /* approaching defense move in battle branch */
  stip_traversal_context_help     /* in help branch */
} stip_traversal_context_type;

#endif
