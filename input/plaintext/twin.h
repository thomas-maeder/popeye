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
  TwinningShift,       /* 11 */
  TwinningSubstitute,  /* 12 */

  TwinningCount   /* 13 */
} TwinningType;

typedef enum
{
  TwinningMirrora1h1,
  TwinningMirrora1a8,
  TwinningMirrora1h8,
  TwinningMirrora8h1,

  TwinningMirrorCount
} TwinningMirrorType;

char *ReadInitialTwin(char *tok, slice_index root_slice_hook);

/* Iterate over the twins of a problem
 * @return token that ended the last twin
 */
char *input_plaintext_twins_handle(char *tok);

void build_solving_machinery(slice_index si);
void build_proof_solving_machinery(slice_index si);
void build_atob_solving_machinery(slice_index si);

void stipulation_completer_solve(slice_index si);

void twin_id_adjuster_solve(slice_index si);

void input_stipulation_solve(slice_index si);

void start_of_solving_machinery_solve(slice_index si);

#endif
