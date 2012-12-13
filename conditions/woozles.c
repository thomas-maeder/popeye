#include "conditions/woozles.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "solving/move_generator.h"
#include "debugging/trace.h"

#include <stdlib.h>

static square  sq_woo_from;
static square  sq_woo_to;
static Side col_woo;

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

  return (flaglegalsquare ? legalsquare : eval_ortho)(sq_departure,sq_arrival,sq_capture);
} /* aux_whx */

static boolean aux_wh(square sq_departure,
                      square sq_arrival,
                      square sq_capture)
{
  if ((flaglegalsquare ? legalsquare : eval_ortho)(sq_departure,sq_arrival,sq_capture)) {
    piece const p= e[sq_woo_from];
    return nbpiece[p]>0
        && (*checkfunctions[abs(p)])(sq_departure, e[sq_woo_from], aux_whx);
  }
  else
    return false;
}

static boolean woohefflibre(square to, square from)
{
  PieNam *pcheck;
  piece p;

  if (rex_wooz_ex && (from == king_square[White] || from == king_square[Black])) {
    return true;
  }

  sq_woo_from= from;
  sq_woo_to= to;
  col_woo= e[from] > vide ? White : Black;

  pcheck = transmpieces[White];
  if (rex_wooz_ex)
    pcheck++;

  while (*pcheck) {
    if (CondFlag[biwoozles] != (col_woo==Black)) {
      p= -*pcheck;
    }
    else {
      p= *pcheck;
    }
    if (nbpiece[p]>0 && (*checkfunctions[*pcheck])(from, p, aux_wh)) {
      return false;
    }
    pcheck++;
  }

  return true;
}

boolean eval_wooheff(square sq_departure, square sq_arrival, square sq_capture) {
  if (flaglegalsquare && !legalsquare(sq_departure,sq_arrival,sq_capture)) {
    return false;
  }
  else {
    return woohefflibre(sq_arrival, sq_departure);
  }
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
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
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

/* Instrument the solvers with Patrol Chess
 * @param si identifies the root slice of the stipulation
 */
void stip_insert_woozles(slice_index si)
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
