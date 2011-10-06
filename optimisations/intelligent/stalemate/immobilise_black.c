#include "optimisations/intelligent/stalemate/immobilise_black.h"
#include "pyint.h"
#include "pydata.h"
#include "pyslice.h"
#include "optimisations/intelligent/stalemate/white_block.h"
#include "optimisations/intelligent/stalemate/black_block.h"
#include "optimisations/intelligent/stalemate/pin_black_piece.h"
#include "options/maxsolutions/maxsolutions.h"
#include "stipulation/temporary_hacks.h"
#include "trace.h"

#include <assert.h>

static immobilisation_state_type const null_immobilisation_state;

immobilisation_state_type * current_immobilisation_state;

static boolean can_white_pin(unsigned int nr_remaining_white_moves)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[White]; i++)
    if (!(white[i].usage!=piece_is_unused
          || white[i].type==cb
          || (white[i].type==pb && nr_remaining_white_moves<moves_to_white_prom[i])))
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static
boolean can_we_block_all_necessary_squares(unsigned int const nr_blocks_needed[nr_sides])
{
  boolean result;
  unsigned int nr_unused_pieces[nr_sides] = { 0, 0 };
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_blocks_needed[White]);
  TraceFunctionParam("%u",nr_blocks_needed[Black]);
  TraceFunctionParamListEnd();

  for (i = 1; i<MaxPiece[Black]; ++i)
    if (black[i].usage==piece_is_unused)
      ++nr_unused_pieces[Black];
  TraceValue("%u\n",nr_unused_pieces[Black]);

  if (nr_unused_pieces[Black]<nr_blocks_needed[Black])
    result = false;
  else
  {
    for (i = 0; i<MaxPiece[White]; ++i)
      if (white[i].usage==piece_is_unused)
        ++nr_unused_pieces[White];
    TraceValue("%u\n",nr_unused_pieces[White]);

    result = (nr_unused_pieces[White]+nr_unused_pieces[Black]
              >=nr_blocks_needed[Black]+nr_blocks_needed[White]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find the most expensive square (if any) that must be blocked by Black
 * @param nr_remaining_black_moves number of remaining black moves
 * @param block_requirement blocking requirements for each square
 * @return * nullsquare more squares need to be blocked than Black can in the
 *                      nr_remaining_black_moves remaining moves
 *         * initsquare no square is required to be blocked by Black
 *         * otherwise: most expensive square that must be blocked by Black
 */
static square find_most_expensive_square_to_be_blocked_by_black(unsigned int nr_remaining_black_moves,
                                                                block_requirement_type const block_requirement[maxsquare+4])
{
  square result = initsquare;
  int max_number_black_moves_to_squares_to_be_blocked = -1;
  unsigned int total_number_black_moves_to_squares_to_be_blocked = 0;

  square const *bnp;
  for (bnp = boardnum; *bnp!=initsquare; ++bnp)
    if (block_requirement[*bnp]==black_block_needed_on_square)
    {
      int const nr_black_blocking_moves = count_nr_black_moves_to_square(*bnp,nr_remaining_black_moves);
      total_number_black_moves_to_squares_to_be_blocked += nr_black_blocking_moves;
      if (total_number_black_moves_to_squares_to_be_blocked>nr_remaining_black_moves)
      {
        result = nullsquare;
        break;
      }
      else if (nr_black_blocking_moves>max_number_black_moves_to_squares_to_be_blocked)
      {
        max_number_black_moves_to_squares_to_be_blocked = nr_black_blocking_moves;
        result = *bnp;
      }
    }


  return result;
}

void intelligent_stalemate_immobilise_black(unsigned int nr_remaining_black_moves,
                                            unsigned int nr_remaining_white_moves,
                                            unsigned int max_nr_allowed_captures_by_black_pieces,
                                            unsigned int max_nr_allowed_captures_by_white_pieces,
                                            stip_length_type n)
{
  immobilisation_state_type immobilisation_state = null_immobilisation_state;

  if (max_nr_solutions_found_in_phase())
    return;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",nr_remaining_black_moves);
  TraceFunctionParam("%u",nr_remaining_white_moves);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_black_pieces);
  TraceFunctionParam("%u",max_nr_allowed_captures_by_white_pieces);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  current_immobilisation_state = &immobilisation_state;
  slice_has_solution(slices[temporary_hack_intelligent_immobilisation_tester[Black]].u.fork.fork);
  current_immobilisation_state = 0;

  assert(immobilisation_state.last_found_trouble_square_status>no_requirement);
  assert(immobilisation_state.nr_of_trouble_makers>0);
  assert(immobilisation_state.positions_of_trouble_makers[immobilisation_state.nr_of_trouble_makers-1]!=initsquare);

  if (immobilisation_state.last_found_trouble_square_status<immobilisation_impossible)
  {
    if (immobilisation_state.last_found_trouble_square_status!=king_block_required
        && can_white_pin(nr_remaining_white_moves))
      intelligent_stalemate_immobilise_by_pinning_any_trouble_maker(nr_remaining_black_moves,
                                                                    nr_remaining_white_moves,
                                                                    max_nr_allowed_captures_by_black_pieces,
                                                                    max_nr_allowed_captures_by_white_pieces,
                                                                    n,
                                                                    &immobilisation_state);

    if (immobilisation_state.last_found_trouble_square_status<pin_required
        && can_we_block_all_necessary_squares(immobilisation_state.nr_blocks_needed))
    {
      square const most_expensive_square_to_be_blocked_by_black
        = find_most_expensive_square_to_be_blocked_by_black(nr_remaining_black_moves,
                                                            immobilisation_state.block_requirement);
      switch (most_expensive_square_to_be_blocked_by_black)
      {
        case nullsquare:
          /* Black doesn't have time to provide all required blocks */
          break;

        case initsquare:
          assert(immobilisation_state.block_requirement[immobilisation_state.last_found_trouble_square]
                 ==white_block_sufficient_on_square);
        {
          /* All required blocks can equally well be provided by White or Black,
           * i.e. they all concern black pawns!
           * We could now try to find the most expensive one, but we assume that
           * there isn't much difference; so simply pick
           * immobilisation_state.last_found_trouble_square.
           */
          intelligent_stalemate_black_block(nr_remaining_black_moves,
                                            nr_remaining_white_moves,
                                            max_nr_allowed_captures_by_black_pieces,
                                            max_nr_allowed_captures_by_white_pieces,
                                            n,
                                            immobilisation_state.last_found_trouble_square);
          intelligent_stalemate_white_block(nr_remaining_black_moves,
                                            nr_remaining_white_moves,
                                            max_nr_allowed_captures_by_black_pieces,
                                            max_nr_allowed_captures_by_white_pieces,
                                            n,
                                            immobilisation_state.last_found_trouble_square);
          break;
        }

        default:
          if (nr_reasons_for_staying_empty[most_expensive_square_to_be_blocked_by_black]==0)
          {
            /* most_expensive_square_to_be_blocked_by_black is the most expensive
             * square among those that Black must block */
            intelligent_stalemate_black_block(nr_remaining_black_moves,
                                              nr_remaining_white_moves,
                                              max_nr_allowed_captures_by_black_pieces,
                                              max_nr_allowed_captures_by_white_pieces,
                                              n,
                                              most_expensive_square_to_be_blocked_by_black);
          }
          break;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
