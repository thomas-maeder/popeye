#include "conditions/marscirce/anti.h"
#include "conditions/marscirce/marscirce.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/walks.h"
#include "position/effects/null_move.h"
#include "position/effects/piece_movement.h"
#include "solving/move_generator.h"
#include "solving/move_effect_journal.h"
#include "stipulation/stipulation.h"
#include "solving/castling.h"
#include "solving/has_solution_type.h"
#include "solving/pipe.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "debugging/assert.h"

circe_variant_type antimars_variant;

static square anti_marscirce_rebirth_square[toppile+1];

/* Determine if the current move is a castling with a reborn rook
 * @param sq_candidate if !=initsquare: position of reborn rook
 * @return if the current move is a castling with a reborn rook: the rebirth square
 *         otherwise: initsquare
 */
static square find_rook_rebirth_for_castling(square sq_candidate)
{
  square result = initsquare;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_candidate);
  TraceFunctionParamListEnd();

  TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture);
  TraceEOL();

  if (sq_candidate!=initsquare
      && min_castling<=move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture
      && move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture<=max_castling)
  {
    piece_walk_type const walk_candidate = get_walk_of_piece_on_square(sq_candidate);
    square const sq_rebirth = circe_regular_rebirth_square(walk_candidate,
                                                           sq_candidate,
                                                           advers(trait[nbply]));

    assert(walk_candidate==standard_walks[Rook]);
    assert(is_square_empty(sq_rebirth));
    result = sq_rebirth;
  }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void anti_marscirce_move_castling_partner_to_rebirth_square_solve(slice_index si)
{
  square const sq_candidate = anti_marscirce_rebirth_square[move_generation_stack[CURRMOVE_OF_PLY(nbply)].id];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",move_generation_stack[CURRMOVE_OF_PLY(nbply)].id);
  TraceSquare(sq_candidate);
  TraceEOL();

  {
    square const sq_rebirth = find_rook_rebirth_for_castling(sq_candidate);

    TraceSquare(sq_rebirth);
    TraceEOL();

    if (sq_rebirth==initsquare)
      move_effect_journal_do_null_effect(move_effect_no_reason);
    else
      move_effect_journal_do_piece_movement(move_effect_reason_phantom_movement,
                                            sq_candidate,sq_rebirth);
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param index index of this generator slice
 */
void anti_mars_circe_second_rebirth_for_castling(slice_index si)
{
  numecoup curr_id = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  /* generate regular moves, including castling without reborn rook: */

  /* make sure that anti_mars_circe_only_castling_after_second_rebirth() will
   * not be confused by a rebirth from an earlier move generation */
  anti_marscirce_rebirth_square[curr_id] = initsquare;

  pipe_move_generation_delegate(si);

  /* make sure that we won't be confused by a possible rebirth from an earlier
   * move generation */
  for (; curr_id<current_move_id[nbply]; ++curr_id)
    anti_marscirce_rebirth_square[curr_id] = initsquare;

  TraceWalk(move_generation_current_walk);
  TraceEnumerator(Side,trait[nbply]);
  TraceValue("%x",TSTCASTLINGFLAGMASK(trait[nbply],k_cancastle));
  TraceEOL();

  /* generate castlings with reborn rook */
  if (is_king(move_generation_current_walk)
      && TSTCASTLINGFLAGMASK(trait[nbply],k_cancastle))
  {
    piece_walk_type const walk_partner = standard_walks[Rook];

    TraceWalk(walk_partner);
    TraceValue("%u",being_solved.number_of_pieces[trait[nbply]][walk_partner]);
    TraceEOL();

    if (being_solved.number_of_pieces[trait[nbply]][walk_partner]>0)
    {
      square const *bnp;
      for (bnp = boardnum; *bnp; ++bnp)
      {
        square const sq_candidate = *bnp;
        piece_walk_type const walk_candidate = get_walk_of_piece_on_square(sq_candidate);

        TraceSquare(sq_candidate);
        TraceWalk(walk_candidate);
        TraceEOL();

        if (walk_candidate==walk_partner
            && TSTFLAG(being_solved.spec[sq_candidate],trait[nbply]))
        {
          square const sq_rebirth = circe_regular_rebirth_square(walk_candidate,
                                                                 sq_candidate,
                                                                 advers(trait[nbply]));

          /* this implies that sq_rebirth!=sq_candidate */
          if (is_square_empty(sq_rebirth))
          {
            Flags const flags = being_solved.spec[sq_candidate];
            numecoup const id_castling_with_reborn_rook = current_move_id[nbply];
            int const file_rebirth = sq_rebirth%nr_files_on_board;
            castling_rights_type const right = file_rebirth==file_rook_queenside ? ra_cancastle : rh_cancastle;

            TraceValue("%u",nbply);
            TraceValue("%u",current_move_id[nbply]);
            TraceValue("%u",id_castling_with_reborn_rook);
            TraceEOL();

            empty_square(sq_candidate);
            occupy_square(sq_rebirth,walk_candidate,flags);
            SETCASTLINGFLAGMASK(trait[nbply],right);

            /* make sure that anti_mars_circe_only_castling_after_second_rebirth() will
             * recognize this rebirth of rook exclusively for castling */
            anti_marscirce_rebirth_square[id_castling_with_reborn_rook] = sq_candidate;

            pipe_move_generation_delegate(si);

            if (current_move_id[nbply]>id_castling_with_reborn_rook)
            {
              TraceText("castling generated - remembering rook rebirth\n");
            }
            else
            {
              TraceText("no castling generated\n");
              anti_marscirce_rebirth_square[id_castling_with_reborn_rook] = initsquare;
            }

            CLRCASTLINGFLAGMASK(trait[nbply],right);
            empty_square(sq_rebirth);
            occupy_square(sq_candidate,walk_candidate,flags);
          }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param index index of this generator slice
 */
void anti_mars_circe_only_castling_after_second_rebirth(slice_index si)
{
  numecoup const id_castling_with_reborn_rook = current_move_id[nbply];

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",id_castling_with_reborn_rook);
  TraceSquare(anti_marscirce_rebirth_square[id_castling_with_reborn_rook]);
  TraceEOL();

  if (anti_marscirce_rebirth_square[id_castling_with_reborn_rook]==initsquare)
  {
    TraceText("not generating castling with reborn rook\n");
    pipe_move_generation_delegate(si);
  }
  else
  {
    TraceText("generating castling with reborn rook\n");
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Prevent Anti-Marscirce from generating null moves
 * @param si identifies the slice
 */
void anti_mars_circe_reject_null_moves(slice_index si)
{
  numecoup const base = CURRMOVE_OF_PLY(nbply);
  numecoup i;
  numecoup new_top = base;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  pipe_move_generation_delegate(si);

  for (i = base+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
  {
    TraceSquare(move_generation_stack[i].departure);
    TraceSquare(move_generation_stack[i].arrival);

    if (move_generation_stack[i].arrival==move_generation_stack[i].departure
        && move_generation_stack[i].capture==no_capture)
    {
      TraceText("rejecting");
    }
    else
    {
      TraceText("accepting");
      ++new_top;
      move_generation_stack[new_top] = move_generation_stack[i];
    }

    TraceEOL();
  }

  SET_CURRMOVE(nbply,new_top);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_no_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STMarsCirceRememberNoRebirth),
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_rebirth(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STAntiMarsCirceRejectNullMoves),
        alloc_pipe(STMarsCirceFixDeparture),
        alloc_pipe(STMarsCirceGenerateFromRebirthSquare),
        alloc_pipe(STMarsCirceRememberRebirth),
        alloc_pipe(STAntiMarsCirceSecondRebirthForCastling),
        alloc_pipe(STAntiMarsCirceOnlyCastlingAfterSecondRebirth),
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise the solving machinery with Anti-Mars Circe
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_antimars(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMarsCirceMoveToRebirthSquare);
  move_effect_journal_register_pre_capture_effect();

  stip_instrument_moves(si,STAntiMarsCirceMoveCastlingPartnerToRebirthSquare);
  move_effect_journal_register_pre_capture_effect();

  solving_instrument_moves_for_piece_generation(si,
                                                nr_sides,
                                                STMoveForPieceGeneratorPathsJoint);

  {
    stip_structure_traversal st;
    move_generator_initialize_instrumentation_for_alternative_paths(&st,nr_sides);
    stip_traverse_structure(si,&st);
  }

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_rebirth);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_no_rebirth);
    stip_traverse_structure(si,&st);
  }

  circe_initialise_solving(si,
                           &antimars_variant,
                           STMoveForPieceGeneratorStandardPath,
                           &slice_insertion_insert_contextually,
                           STAntimarsCirceConsideringRebirth);
  circe_instrument_solving(si,
                           STAntimarsCirceConsideringRebirth,
                           STCirceDeterminedRebirth,
                           alloc_pipe(STMarscirceRemoveReborn));

  if (antimars_variant.rebirth_reason==move_effect_reason_rebirth_choice)
    circe_instrument_solving(si,
                             STAntimarsCirceConsideringRebirth,
                             STAntimarsCirceConsideringRebirth,
                             alloc_pipe(STMoveGenerationPostMoveIterator));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
