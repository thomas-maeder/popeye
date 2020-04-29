#include "conditions/bolero.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @note the piece on the departure square need not necessarily have walk p
 */
void bolero_generate_non_captures(slice_index si)
{
  square const sq_departure = curr_generation->departure;
  piece_walk_type const regular_walk = get_walk_of_piece_on_square(sq_departure);

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_pawn(regular_walk) || TSTFLAG(being_solved.spec[sq_departure],Royal))
    pipe_move_generation_delegate(si);
  else
  {
    square const square_1 = trait[nbply]==White ? square_a1 : square_a8;
    unsigned int const sq_departure_column = sq_departure%onerow - nr_of_slack_files_left_of_board;
    square const sq_relevant = square_1+sq_departure_column;
    piece_walk_type const non_capture_walk = game_array.board[sq_relevant];

    TraceSquare(square_1);
    TraceSquare(sq_departure);
    TraceValue("%u",sq_departure_column);
    TraceSquare(sq_relevant);
    TraceWalk(non_capture_walk);
    TraceEOL();

    pipe_move_generation_different_walk_delegate(si,non_capture_walk);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_no_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] =
    {
        alloc_pipe(STBoleroGenerateNonCapturesWalkByWalk),
        alloc_pipe(STMoveGeneratorRejectCaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_capture(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  {
    slice_index const prototypes[] = {
        alloc_pipe(STMoveGeneratorRejectNoncaptures)
    };
    enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
    slice_insertion_insert_contextually(si,st->context,prototypes,nr_prototypes);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Inialise thet solving machinery with Mars Circe
 * @param si identifies the root slice of the solving machinery
 */
void solving_initialise_bolero(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  move_generator_instrument_for_alternative_paths(si,nr_sides);

  {
    stip_structure_traversal st;
    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorStandardPath,
                                             &instrument_no_capture);
    stip_structure_traversal_override_single(&st,
                                             STMoveForPieceGeneratorAlternativePath,
                                             &instrument_capture);
    stip_traverse_structure(si,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
