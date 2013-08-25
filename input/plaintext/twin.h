#if !defined(INPUT_PLAINTEXT_TWIN_H)
#define INPUT_PLAINTEXT_TWIN_H

#include "stipulation/stipulation.h"
#include "input/plaintext/token.h"

typedef enum
{
  TwinningMove,         /* 0 */
  TwinningExchange,     /* 1 */
  TwinningStip,         /* 2 */
  TwinningStructStip,   /* 3 */
  TwinningAdd,          /* 4 */
  TwinningRemove,       /* 5 */
  TwinningContinued,    /* 6 */
  TwinningRotate,       /* 7 */
  TwinningCond,         /* 8 */
  TwinningPolish,       /* 9 */
  TwinningMirror,      /* 10 */
  TwinningMirra1h1,    /* 11 */
  TwinningMirra1a8,    /* 12 */
  TwinningMirra1h8,    /* 13 */
  TwinningMirra8h1,    /* 14 */
  TwinningShift,       /* 15 */
  TwinningSubstitute,  /* 16 */
  TwinningForsyth,     /* 17 */

  TwinningCount   /* 17 */
} TwinningType;

Token ReadTwin(Token tk, slice_index root_slice_hook);

/* Iterate over the twins of a problem
 * @prev_token token that ended the previous twin
 * @return token that ended the current twin
 */
Token iterate_twins(Token prev_token);

#endif
