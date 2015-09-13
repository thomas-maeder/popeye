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

/* Iterate over the twins of a problem
 */
void input_plaintext_twins_handle(slice_index start);

void input_plaintext_initial_twin_reader_solve(slice_index si);

void stipulation_copier_solve(slice_index si);
void build_proof_solving_machinery(slice_index si);
void build_atob_solving_machinery(slice_index si);

void solving_machinery_intro_builder_solve(slice_index si);

void stipulation_completer_solve(slice_index si);

void twin_id_adjuster_solve(slice_index si);

void start_of_current_twin_solve(slice_index si);

#endif
