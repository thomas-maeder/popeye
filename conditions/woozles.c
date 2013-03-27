#include "conditions/woozles.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <stdlib.h>

static square  sq_woo_from;
static square  sq_woo_to;

static boolean aux_whx(square sq_departure,
                       square sq_arrival,
                       square sq_capture)
{
  if (sq_departure != sq_woo_from)
    return false;

  /* sq_departure == sq_woo_from */
  if (CondFlag[heffalumps]) {
    int cd1= sq_departure%onerow - sq_arrival%onerow;
    int rd1= sq_departure/onerow - sq_arrival/onerow;
    int cd2= sq_woo_to%onerow - sq_departure%onerow;
    int rd2= sq_woo_to/onerow - sq_departure/onerow;
    int t= 7;

    if (cd1 != 0)
      t= abs(cd1);
    if (rd1 != 0 && t > abs(rd1))
      t= abs(rd1);

    while (!(cd1%t == 0 && rd1%t == 0))
      t--;
    cd1= cd1/t;
    rd1= rd1/t;

    t= 7;
    if (cd2 != 0)
      t= abs(cd2);
    if (rd2 != 0 && t > abs(rd2))
      t= abs(rd2);

    while (!(cd2%t == 0 && rd2%t == 0))
      t--;

    cd2= cd2/t;
    rd2= rd2/t;

    if (!(  (cd1 == cd2 && rd1 == rd2)
            || (cd1 == -cd2 && rd1 == -rd2)))
    {
      return false;
    }
  }

  return validate_observation_geometry(sq_departure,sq_arrival,sq_capture);
} /* aux_whx */

static boolean aux_wh(square sq_departure,
                      square sq_arrival,
                      square sq_capture)
{
  if (validate_observation_geometry(sq_departure,sq_arrival,sq_capture))
  {
    piece const p = e[sq_woo_from];
    return nbpiece[p]>0
        && (*checkfunctions[abs(p)])(sq_departure,e[sq_woo_from],&aux_whx);
  }
  else
    return false;
}

static boolean woohefflibre(square to, square from)
{
  PieNam *pcheck;
  Side const col_woo = e[from]>vide ? White : Black;

  if (rex_wooz_ex && from==king_square[col_woo])
    return true;

  sq_woo_from = from;
  sq_woo_to = to;

  pcheck = transmpieces[White];
  if (rex_wooz_ex)
    ++pcheck;

  while (*pcheck)
  {
    piece const p = CondFlag[biwoozles]!=(col_woo==Black) ? -*pcheck : *pcheck;

    if (nbpiece[p]>0 && (*checkfunctions[*pcheck])(from,p,&aux_wh))
      return false;
    else
      ++pcheck;
  }

  return true;
}

/* Validate an observation according to Woozles
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean woozles_heffalumps_validate_observation(square sq_observer,
                                                square sq_landing,
                                                square sq_observee)
{
  return woohefflibre(sq_landing,sq_observer);
}

static boolean is_not_illegal_capture(square sq_departure,
                                      square sq_arrival,
                                      square sq_capture)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceFunctionParamListEnd();

  result = !(e[sq_capture]!=vide
             && !woohefflibre(sq_arrival, sq_departure));

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type woozles_remove_illegal_captures_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  move_generator_filter_moves(&is_not_illegal_capture);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_remover(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototype = alloc_pipe(STWoozlesRemoveIllegalCaptures);
    branch_insert_slices_contextual(si,st->context,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument solving in Woozles
 * @param si identifies the root slice of the stipulation
 */
void woozles_initialise_solving(slice_index si)
{
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,0);
  stip_structure_traversal_override_single(&st,
                                           STDoneGeneratingMoves,
                                           &insert_remover);
  stip_traverse_structure(si,&st);

  register_observation_validator(&woozles_heffalumps_validate_observation);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
