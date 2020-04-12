#include "pieces/attributes/magic.h"
#include "pieces/walks/roses.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/move.h"
#include "position/effects/side_change.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/fork.h"
#include "optimisations/orthodox_check_directions.h"
#include "pieces/pieces.h"
#include "output/plaintext/message.h"

#include "debugging/trace.h"
#include "debugging/assert.h"

#include <stdlib.h>

/* magic pieces */
enum
{
  magicviews_size = 10000
};

typedef struct
{
  square pos_viewed;
  PieceIdType viewedid;
  PieceIdType magicpieceid;
  square line_start;
  square line_end;
} magicview_type;

static magicview_type magicviews[magicviews_size];

static unsigned int magic_views_top[maxply + 1];

static ply stack_pointer;

static void PushMagicView(square pos_viewed, square pos_magic, square start, square end)
{
  unsigned int const top = magic_views_top[stack_pointer];

  TraceFunctionEntry(__func__);
  TraceSquare(pos_viewed);
  TraceSquare(pos_magic);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParamListEnd();

  assert(magic_views_top[stack_pointer]<magicviews_size);

  magicviews[top].pos_viewed = pos_viewed;
  magicviews[top].viewedid = GetPieceId(being_solved.spec[pos_viewed]);
  magicviews[top].magicpieceid = GetPieceId(being_solved.spec[pos_magic]);
  magicviews[top].line_start = start;
  magicviews[top].line_end = end;
  ++magic_views_top[stack_pointer];

  TraceValue("%u",stack_pointer);
  TraceValue("%u",magic_views_top[stack_pointer]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Can a specific type of (fairy) piece be magic?
 * @param p type of piece
 * @return true iff pieces of type p can be magic
 */
boolean magic_is_piece_supported(piece_walk_type p)
{
  if (p>=Hunter0)
    return false;
  else
    switch (p)
    {
      case UbiUbi:
      case Archbishop:
      case ReflectBishop:
      case Cardinal:
      case BouncyKnight:
      case CAT:
      case Orphan:
      case Friend:
      case BouncyNightrider:
        return false;

      default:
        return true;
    }
}

static boolean are_we_finding_magic_views[maxply+1];
static unsigned int prev_observation_context[maxply+1];

static void identify_straight_line(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

  vec_index_type const idx = interceptable_observation[observation_context].vector_index1;
  numvec const dir = vec[idx];

  /* we identify straight lines by the two virtual squares just outside of the
   * board
   */

  square start = sq_observer;
  square end = sq_observee;

  do
  {
    start += dir;
  } while (!is_square_blocked(start));

  end = sq_observee;
  do
  {
    end -= dir;
  } while (!is_square_blocked(end));

  PushMagicView(sq_observee,sq_observer,start,end);
}

static void identify_circular_line(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  rose_rotation_sense const sense = interceptable_observation[observation_context].auxiliary;
  vec_index_type idx = interceptable_observation[observation_context].vector_index1;
  square sq_curr = sq_observee;

  square start = sq_observee;
  square end = start+vec[idx];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* we identify circular lines by the lowest square (start) and the one
   * immediately following it (end)
   */

  do
  {
    sq_curr += vec[idx];
    // TODO does this overflow work on all implementations?
    assert(abs(sense)==1);
    assert(idx>0 || sense>0);
    idx += (unsigned int)sense;

    if (start>sq_curr)
    {
      start = sq_curr;
      end = start+vec[idx];
    }
  } while (sq_curr!=sq_observee);

  PushMagicView(sq_observee,sq_observer,start,end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void identify_zigzag_line(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type const idx_zig = interceptable_observation[observation_context].vector_index1;
  vec_index_type const idx_zag = interceptable_observation[observation_context].auxiliary;
  square sq_curr = sq_observee+vec[idx_zig];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* we identify zigzag lines by the two first off the board squares
   */

  while (true)
  {
    if (is_square_blocked(sq_curr))
    {
      PushMagicView(sq_observee,sq_observer,sq_curr+vec[idx_zag],sq_curr);
      break;
    }
    else
    {
      sq_curr += vec[idx_zag];

      if (is_square_blocked(sq_curr))
      {
        PushMagicView(sq_observee,sq_observer,sq_curr+vec[idx_zig],sq_curr);
        break;
      }
      else
        sq_curr += vec[idx_zig];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void identify_doublehopper_line(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  vec_index_type const idx_firstleg = interceptable_observation[observation_context].vector_index1;
  numvec const dir_firstleg = vec[idx_firstleg];
  vec_index_type const sq_intermediate = interceptable_observation[observation_context].auxiliary;
  square sq_curr = sq_intermediate+dir_firstleg;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* we identify doublehopper lines by the intermediate square and the square
   * just off the board on the first leg
   */

  while (!is_square_blocked(sq_curr))
    sq_curr += dir_firstleg;

  PushMagicView(sq_observee,sq_observer,sq_curr,sq_intermediate);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void identify_leaper_pseudoline(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;
  square const sq_observee = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;
  PushMagicView(sq_observee,sq_observer,sq_observer,sq_observee);
}

static void identify_combined_rider_leaper_line(void)
{
  if (observation_context==prev_observation_context[nbply])
    identify_leaper_pseudoline();
  else
    identify_straight_line();
}

static void identify_line(void)
{
  square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_observer);
  TraceWalk(being_solved.board[sq_observer]);
  TraceEOL();
  switch (being_solved.board[sq_observer])
  {
    /* TODO simplify using classes? */
    case Rook:
    case Queen:
    case Bishop:
    case NightRider:
    case Waran:
    case Camelrider:
    case Zebrarider:
    case Gnurider:
    case SuperBerolinaPawn:
    case SuperPawn:
    case RookHunter:
    case BishopHunter:
    case WesirRider:
    case FersRider:
    case Elephant:
    case Lion:
    case RookLion:
    case BishopLion:
    case NightRiderLion:
    case Pao:
    case Leo:
    case Vao:
    case Nao:
    case Locust:
    case NightLocust:
    case BishopLocust:
    case RookLocust:
    case Sirene:
    case Triton:
    case Nereide:
    case Kangaroo:
    case KangarooLion:
    case Rabbit:
    case Kao:
    case Elk:
    case RookMoose:
    case BishopMoose:
    case Eagle:
    case RookEagle:
    case BishopEagle:
    case Sparrow:
    case RookSparrow:
    case BishopSparrow:
    case Marguerite:
    case EquiHopper:
    case Orix:
    case EdgeHog:
    case Grasshopper:
    case NightriderHopper:
    case CamelRiderHopper:
    case ZebraRiderHopper:
    case GnuRiderHopper:
    case RookHopper:
    case BishopHopper:
    case ContraGras:
    case GrassHopper2:
    case GrassHopper3:
    case Bob:
    case EquiStopper:
    case MaoRider:
    case MoaRider:
    case MaoRiderLion:
    case MoaRiderLion:
    case Dolphin:
    case Bouncer :
    case RookBouncer:
    case BishopBouncer:
      identify_straight_line();
      break;

    case Rose:
    case RoseLion:
    case RoseHopper:
    case RoseLocust:
    case Rao:
      identify_circular_line();
      break;

    case SpiralSpringer:
    case DiagonalSpiralSpringer:
    case SpiralSpringer40:
    case SpiralSpringer20:
    case SpiralSpringer33:
    case SpiralSpringer11:
    case BoyScout:
    case GirlScout:
      identify_zigzag_line();
      break;

    case DoubleGras:
    case DoubleRookHopper:
    case DoubleBishopper:
      identify_doublehopper_line();
      break;

    case Amazone:
    case Empress:
    case Princess:
    case Gryphon:
    case Ship:
    case Gral:
    case Scorpion:
    case Querquisite:
    case MarineShip:
      identify_combined_rider_leaper_line();
      break;

    case King:
    case Poseidon:
    case ErlKing:
    case Knight:
    case Zebra:
    case Camel:
    case Giraffe:
    case RootFiftyLeaper:
    case Bucephale:
    case Wesir:
    case Alfil:
    case Fers:
    case Dabbaba:
    case Gnu:
    case Antilope:
    case Squirrel:
    case Okapi:
    case Leap37:
    case Leap16:
    case Leap24:
    case Leap35:
    case Leap15:
    case Leap25:
    case Bison:
    case Zebu:
    case Leap36:
    case Mao:
    case Moa:
    case NonStopEquihopper:
    case NonStopOrix:
    case NonstopEquiStopper:
    case KingHopper:
    case KnightHopper:
    case MarineKnight:

    case Pawn:
    case BerolinaPawn:
    case ReversePawn:
    case ChinesePawn:
    case MarinePawn:

    case RadialKnight:
    case Treehopper:
    case Leafhopper :
    case GreaterTreehopper:
    case GreaterLeafhopper:

    case Skylla:
    case Charybdis:

    case Dragon:
      identify_leaper_pseudoline();
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

boolean magic_enforce_observer(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (are_we_finding_magic_views[nbply])
  {
    square const sq_observer = move_generation_stack[CURRMOVE_OF_PLY(nbply)].departure;

    if (TSTFLAG(being_solved.spec[sq_observer],Magic))
    {
       if (pipe_validate_observation_recursive_delegate(si))
        identify_line();

      result = false; /* we need all views */
    }
    else
      result = false;
  }
  else
    result = pipe_validate_observation_recursive_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void PushMagicViewsByOnePiece(piece_walk_type pi_magic)
{
  square const *pos_viewed;

  TraceFunctionEntry(__func__);
  TraceWalk(pi_magic);
  TraceFunctionParamListEnd();

  for (pos_viewed = boardnum; *pos_viewed; pos_viewed++)
    if (get_walk_of_piece_on_square(*pos_viewed)>Invalid
        && !TSTFLAGMASK(being_solved.spec[*pos_viewed],BIT(Magic)|BIT(Royal))
        && !is_piece_neutral(being_solved.spec[*pos_viewed]))
    {
      replace_observation_target(*pos_viewed);
      observing_walk[nbply] = pi_magic;
      /* ignore return value - it's ==false */
      fork_is_square_observed_nested_delegate(temporary_hack_is_square_observed_specific[trait[nbply]],
                                              EVALUATE(observation));
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef unsigned int mark_type;

static mark_type current_mark = 0;
static mark_type walk_tried[nr_piece_walks] = { 0 };

static void PushMagicViewsByOneSide(Side side)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  trait[nbply] = side;

  if (current_mark==UINT_MAX)
  {
    piece_walk_type p;
    for (p = King; p<nr_piece_walks; ++p)
      walk_tried[p] = 0;

    current_mark = 1;
  }
  else
    ++current_mark;

  {
    square const *pos_magic;
    for (pos_magic = boardnum; *pos_magic; pos_magic++)
      /* insisting on TSTFLAG(being_solved.spec[*pos_magic],Magic) would prevent magic pieces
       * from working properly in conditions such as Annan Chess
       */
      if (TSTFLAG(being_solved.spec[*pos_magic],side))
      {
        piece_walk_type const pi_magic = get_walk_of_piece_on_square(*pos_magic);
        if (walk_tried[pi_magic]!=current_mark)
        {
          walk_tried[pi_magic] = current_mark;
          PushMagicViewsByOnePiece(pi_magic);
        }
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void PushMagicViews(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  magic_views_top[stack_pointer] = magic_views_top[stack_pointer-1];

  siblingply(no_side);
  push_observation_target(initsquare);
  prev_observation_context[nbply] = observation_context;

  are_we_finding_magic_views[nbply] = true;

  PushMagicViewsByOneSide(White);
  PushMagicViewsByOneSide(Black);

  are_we_finding_magic_views[nbply] = false;

  /* TODO: remove double views by neutral magic pieces
   * apply same logic as for cross-eyed pieces? */

  finply();

  TraceValue("%u",nbply);
  TraceValue("%u",magic_views_top[nbply]);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean find_view(ply ply_id, int j)
{
  PieceIdType const currid = magicviews[j].viewedid;
  PieceIdType const magicpieceid = magicviews[j].magicpieceid;
  square const start = magicviews[j].line_start;
  square const end = magicviews[j].line_end;
  boolean result = false;
  unsigned int k;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",ply_id);
  TraceFunctionParam("%d",j);
  TraceFunctionParamListEnd();

  for (k = magic_views_top[stack_pointer-2]; k<magic_views_top[stack_pointer-1]; ++k)
    if (magicviews[k].viewedid==currid
        && magicviews[k].magicpieceid==magicpieceid
        && magicviews[k].line_start==start
        && magicviews[k].line_end==end)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int count_changed_views(square sq_viewed)
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_viewed);
  TraceFunctionParamListEnd();

  for (i = magic_views_top[stack_pointer-1]; i<magic_views_top[stack_pointer]; ++i)
    if (magicviews[i].pos_viewed==sq_viewed
        && !find_view(stack_pointer-1,i))
      ++result;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void ChangeMagic(void)
{
  square const *bnp;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp = boardnum; *bnp; ++bnp)
    /* only change if viewee suffers odd-no. new views */
    if (count_changed_views(*bnp)%2==1)
    {
      assert(!is_piece_neutral(being_solved.spec[*bnp]));
      move_effect_journal_do_side_change(move_effect_reason_magic_piece,*bnp);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void magic_views_initialiser_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++stack_pointer;
  assert(nbply==ply_retro_move);
  PushMagicViews();
  pipe_solve_delegate(si);
  --stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
void magic_pieces_recolorer_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  ++stack_pointer;
  PushMagicViews();
  ChangeMagic();
  pipe_solve_delegate(si);
  --stack_pointer;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void solving_insert_magic_pieces_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMagicPiecesRecolorer);

  /* make sure that we compare the correct magic views in threats */
  stip_instrument_dummy_moves(si,STMagicPiecesRecolorer);

  {
    slice_index const prototype = alloc_pipe(STMagicViewsInitialiser);
    slice_insertion_insert(si,&prototype,1);
  }

  stip_instrument_observation_validation(si,nr_sides,STMagicPiecesObserverEnforcer);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#ifdef DEBUG
static void WriteMagicViews(int ply)
{
  int i;
  for (i= magictop[ply-1]; i < magictop[ply]; i++)
  {
    WriteSquare(stdout,magicviews[i].pos_viewed);
    fputc(' ',stdout);
    WriteSquare(stdout,magicviews[i].viewedid);
    fputc(' ',stdout);
    WriteSquare(stdout,magicviews[i].magicpieceid);
    fputc(' ',stdout);
    fprintf(stdout, "%i", magicviews[i].line_identifier);
    fputc('\n',stdout);
  }
}
#endif
