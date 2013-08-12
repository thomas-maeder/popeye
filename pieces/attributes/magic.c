#include "pieces/attributes/magic.h"
#include "pydata.h"
#include "pymsg.h"
#include "pieces/walks/roses.h"
#include "pieces/attributes/neutral/neutral.h"
#include "position/pieceid.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/branch.h"
#include "stipulation/move.h"
#include "solving/move_effect_journal.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "debugging/trace.h"

#include <assert.h>

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

static void PushMagicView(square pos_viewed, square pos_magic, square start, square end)
{
  unsigned int const top = magic_views_top[nbply-1];

  TraceFunctionEntry(__func__);
  TraceSquare(pos_viewed);
  TraceSquare(pos_magic);
  TraceSquare(start);
  TraceSquare(end);
  TraceFunctionParamListEnd();

  assert(magic_views_top[nbply-1]<magicviews_size);

  magicviews[top].pos_viewed = pos_viewed;
  magicviews[top].viewedid = GetPieceId(spec[pos_viewed]);
  magicviews[top].magicpieceid = GetPieceId(spec[pos_magic]);
  magicviews[top].line_start = start;
  magicviews[top].line_end = end;
  ++magic_views_top[nbply-1];

  TraceValue("%u",nbply);
  TraceValue("%u\n",magic_views_top[nbply-1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Can a specific type of (fairy) piece be magic?
 * @param p type of piece
 * @return true iff pieces of type p can be magic
 */
boolean magic_is_piece_supported(PieNam p)
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
        return false;

      default:
        return true;
    }
}

static void identify_straight_line(void)
{
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_observee = move_generation_stack[current_move[nbply]-1].capture;

  vec_index_type const idx = interceptable_observation[observation_context].vector_index;
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
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_observee = move_generation_stack[current_move[nbply]-1].capture;
  rose_rotation_sense const sense = interceptable_observation[observation_context].auxiliary;
  vec_index_type idx = interceptable_observation[observation_context].vector_index;
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
    idx += sense;

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
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_observee = move_generation_stack[current_move[nbply]-1].capture;
  vec_index_type const idx_zig = interceptable_observation[observation_context].vector_index;
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

static void identify_line(void)
{
  square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;
  square const sq_observee = move_generation_stack[current_move[nbply]-1].capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(sq_observer);
  TraceSquare(sq_observee);
  TracePiece(e[sq_observer]);
  TraceText("\n");
  switch (e[sq_observer])
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

    /* TODO consider again */
    case BouncyNightrider:
    case DoubleGras:
    case DoubleRookHopper:
    case DoubleBishopper:
    case Bouncer :
    case RookBouncer:
    case BishopBouncer :

    /* TODO combined pieces */
    case Amazone:
    case Empress:
    case Princess:
    case Dragon:
    case Gryphon:
    case Ship:
    case Gral:
    case Scorpion:
    case Dolphin:
    case Querquisite:
    case MarineShip:
      PushMagicView(sq_observee,sq_observer,sq_observer,sq_observee);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square current_magic_pos[maxply+1];

boolean magic_enforce_observer(slice_index si)
{
  square const sq_magician = current_magic_pos[nbply];
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (sq_magician==initsquare)
    /* we are not detecting magic views */
    result = validate_observation_recursive(slices[si].next1);
  else
  {
    square const sq_observer = move_generation_stack[current_move[nbply]-1].departure;

    if (sq_magician==sq_observer
        && validate_observation_recursive(slices[si].next1))
      identify_line();

    result = false; /* we need all views */
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void PushMagicViewsByOnePiece(square pos_magic)
{
  PieNam const pi_magic = get_walk_of_piece_on_square(pos_magic);
  square const *pos_viewed;

  TraceFunctionEntry(__func__);
  TraceSquare(pos_magic);
  TraceFunctionParamListEnd();

  for (pos_viewed = boardnum; *pos_viewed; pos_viewed++)
    if (get_walk_of_piece_on_square(*pos_viewed)>Invalid
        && !TSTFLAGMASK(spec[*pos_viewed],BIT(Magic)|BIT(Royal))
        && !is_piece_neutral(spec[*pos_viewed]))
    {
      /* for each non-magic piece
         (n.b. check *pos_magic != *pos_viewed redundant above) */
      move_generation_stack[current_move[nbply]-1].capture = *pos_viewed;
      if (pi_magic<Queen || pi_magic>Bishop
          || CheckDir[pi_magic][*pos_viewed-pos_magic]!=0)
      {
        observing_walk[nbply] = pi_magic;
        current_magic_pos[nbply] = pos_magic;
        /* ignore return value - it's ==false */
        (*checkfunctions[pi_magic])(&validate_observation);
        current_magic_pos[nbply] = initsquare;
      }
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void PushMagicViews(void)
{
  square const *pos_magic;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  magic_views_top[nbply] = magic_views_top[nbply-1];

  siblingply(trait[nbply]);
  current_move[nbply] = current_move[nbply-1]+1;

  for (pos_magic = boardnum; *pos_magic; pos_magic++)
    if (TSTFLAG(spec[*pos_magic], Magic))
    {
      if (TSTFLAG(spec[*pos_magic],White))
      {
        trait[nbply] = White;
        PushMagicViewsByOnePiece(*pos_magic);
      }
      if (TSTFLAG(spec[*pos_magic],Black))
      {
        trait[nbply] = Black;
        PushMagicViewsByOnePiece(*pos_magic);
      }
      /* TODO: remove double views my neutral magic pieces
       * apply same logic as for cross-eyed pieces? */
    }

  finply();

  TraceValue("%u",nbply);
  TraceValue("%u\n",magic_views_top[nbply]);

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

  for (k = magic_views_top[ply_id-1]; k<magic_views_top[ply_id]; ++k)
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

  for (i = magic_views_top[nbply-1]; i<magic_views_top[nbply]; ++i)
    if (magicviews[i].pos_viewed==sq_viewed
        && !find_view(parent_ply[nbply],i))
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
      assert(!is_piece_neutral(spec[*bnp]));
      move_effect_journal_do_side_change(move_effect_reason_magic_piece,*bnp);
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
stip_length_type magic_views_initialiser_solve(slice_index si,
                                                stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(nbply==1);
  PushMagicViews();
  result = solve(slices[si].next1,n);

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
stip_length_type magic_pieces_recolorer_solve(slice_index si,
                                               stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  PushMagicViews();
  ChangeMagic();
  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_magic_pieces_recolorers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMagicPiecesRecolorer);

  /* make sure that we compare the correct magic views in threats */
  stip_instrument_dummy_moves(si,STMagicPiecesRecolorer);

  {
    slice_index const prototype = alloc_pipe(STMagicViewsInitialiser);
    branch_insert_slices(si,&prototype,1);
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
    char buf[10];
    WriteSquare(magicviews[i].pos_viewed);
    StdChar(' ');
    WriteSquare(magicviews[i].viewedid);
    StdChar(' ');
    WriteSquare(magicviews[i].magicpieceid);
    StdChar(' ');
    sprintf(buf, "%i", magicviews[i].line_identifier);
    StdString(buf);
    StdChar('\n');
  }
}
#endif
