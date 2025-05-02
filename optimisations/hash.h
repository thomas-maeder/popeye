/******************** MODIFICATIONS to pyhash.h **************************
**
** Date       Who  What
**
** 1994            Original
**
**************************** End of List ******************************/

#if !defined(OPTIMISATIONS_HASH_H)
#define OPTIMISATIONS_HASH_H

#include "DHT/dhtbcmem.h"
#include "pieces/pieces.h"
#include "solving/machinery/solve.h"
#include "position/underworld.h"
#include "position/position.h"
#include "position/board.h"
#include "solving/ply.h"
#include "conditions/bgl.h"
#include "conditions/fuddled_men.h"

#if defined(TESTHASH)
#       if !defined(HASHRATE)
#               define HASHRATE
#       endif   /* !HASHRATE */
#endif  /* TESTHASH */

/* typedefs */
typedef unsigned char byte;

/* Time for some math -- let's figure out the maximum number of bytes a position encoding may take up.
   If it's a proofgame then ProofEncode will be used.  Otherwise, SmallEncode or LargeEncode will be chosen,
   the former if TellSmallEncodePosLeng <= TellLargeEncodePosLeng, the latter otherwise.
   - ProofEncode <=   nr_rows_on_board
                    + "num nonempty squares (found by looping over nr_rows_on_board and nr_files_on_board)" * max(ProofLargeEncodePiece, ProofSmallEncodePiece)
                    + 1
                    + nr_ghosts * SmallEncodePiece
                    + CommonEncode
   - LargeEncode =   nr_rows_on_board
                   + "num nonempty squares (found by looping over nr_rows_on_board and nr_files_on_board)" * LargeEncodePiece
                   + nr_ghosts * (2 + bytes_per_spec)
                   + CommonEncode
   - SmallEncode =   "num nonempty squares (found by looping over nr_rows_on_board and nr_files_on_board)" * SmallEncodePiece
                   + nr_ghosts * SmallEncodePiece
                   + CommonEncode
   - TellLargeEncodePosLeng = TellCommonEncodePosLeng(len, nbr_p)
                              with
                                  len =   8
                                        + "num nonempty squares (found by looping boardnum until 0 is found)" * bytes_per_piece
                                        + !!CondFlag[BGL] * (sizeof BGL_values[White] + sizeof BGL_values[Black])
                                        + nr_ghosts * bytes_per_piece
                                  nbr_p = "num nonempty squares (found by looping boardnum until 0 is found)"
   - TellSmallEncodePosLeng = TellCommonEncodePosLeng(len, nbr_p)
                              with
                                  len =   "num nonempty squares (found by looping boardnum until 0 is found)" * (1 + bytes_per_piece)
                                        + nr_ghosts * bytes_per_piece
                                  nbr_p = "num nonempty squares (found by looping boardnum until 0 is found)"
   TellCommonEncodePosLeng(len, np) is a strictly increasing function of len.  SmallEncode will therefore be chosen iff
   "num nonempty squares" <= 8 + !!CondFlag[BGL] * (sizeof BGL_values[White] + sizeof BGL_values[Black]).
   Therefore:
   - If SmallEncode is chosen then "num nonempty squares" <= min((nr_rows_on_board * nr_files_on_board), (8 + sizeof BGL_values[White] + sizeof BGL_values[Black])).
   - If LargeEncode is chosen then 8 < "num nonempty squares" <= (nr_rows_on_board * nr_files_on_board).
   Substituting we find that when chosen:
   - SmallEncode <=   min((nr_rows_on_board * nr_files_on_board), (8 + sizeof BGL_values[White] + sizeof BGL_values[Black])) * SmallEncodePiece
                    + nr_ghosts * SmallEncodePiece
                    + CommonEncode
   - LargeEncode <=   nr_rows_on_board
                    + (nr_rows_on_board * nr_files_on_board) * LargeEncodePiece
                    + nr_ghosts * (2 + bytes_per_spec)
                    + CommonEncode
   - ProofEncode <=   nr_rows_on_board
                    + (nr_rows_on_board * nr_files_on_board) * max(ProofLargeEncodePiece, ProofSmallEncodePiece)
                    + 1
                    + nr_ghosts * SmallEncodePiece
                    + CommonEncode
   Meanwhile, inspection reveals that:
   - ProofLargeEncodePiece = 2
   - ProofSmallEncodePiece <= 1
   - SmallEncodePiece <= 1 + 1 + bytes_per_spec
   - LargeEncodePiece <= 1 + bytes_per_spec
   - CommonEncode <=   2 + 2 + 1 + 1 + 1
                     + being_solved.number_of_imitators
                     + 1 + 1 + 3 + 1
                     + bytes_per_spec
                     + 1 + 2
                     + en_passant_end[nbply] - en_passant_end[nbply-1]
                     + 1
                     + sizeof BGL_values[White] + sizeof BGL_values[Black]
                     + 2
                     + sizeof fuddled
   Substituting yields:
   - SmallEncode <=   min((nr_rows_on_board * nr_files_on_board), (8 + sizeof BGL_values[White] + sizeof BGL_values[Black])) * (1 + 1 + bytes_per_spec)
                    + nr_ghosts * (1 + 1 + bytes_per_spec)
                    + CommonEncode
   - LargeEncode <=   nr_rows_on_board
                    + (nr_rows_on_board * nr_files_on_board) * (1 + bytes_per_spec)
                    + nr_ghosts * (2 + bytes_per_spec)
                    + CommonEncode
   - ProofEncode <=   nr_rows_on_board
                    + (nr_rows_on_board * nr_files_on_board) * 2
                    + 1
                    + nr_ghosts * (2 + bytes_per_spec)
                    + CommonEncode
   We need to bound the remaining non-constants.  We have
   - nr_ghosts <= underworld_capacity
   - bytes_per_spec <= 4
   - being_solved.number_of_imitators <= maxinum
   en_passant_end[nbply] - en_passant_end[nbply-1] requires a bit more thought.  It should be an upper
   bound on the number of en passant squares stored per ply.  For now we'll assume that
   - en_passant_end[nbply] - en_passant_end[nbply-1] <= nr_rows_on_board - 2
   as (nr_rows_on_board - 2) is the number of squares that a pawn jumping from the first rank to the
   last rank would skip over.
   With all of the above, we can determine the maximum bytes that an encoding should take up.  We'll
   let the compiler perform the arithmetic.
*/

enum {
  MAX_NR_GHOSTS = underworld_capacity,
  MAX_BYTES_PER_SPEC = 4,
  MAX_NUMBER_OF_IMITATORS = maxinum,
  MAX_EN_PASSANT_END_DIFFERENCE = nr_rows_on_board - 2, /* TODO: Is this a safe maximum?  Can we get away with a smaller value? */
  COMMONENCODE_MAX =   2 + 2 + 1 + 1 + 1
                     + MAX_NUMBER_OF_IMITATORS
                     + 1 + 1 + 3 + 1
                     + MAX_BYTES_PER_SPEC
                     + 1 + 2
                     + MAX_EN_PASSANT_END_DIFFERENCE
                     + 1
                     + sizeof BGL_values[White] + sizeof BGL_values[Black]
                     + 2
                     + sizeof fuddled,
  NUM_NONEMPTY_SMALL_FIRST = (nr_rows_on_board * nr_files_on_board),
  NUM_NONEMPTY_SMALL_SECOND = (8 + sizeof BGL_values[White] + sizeof BGL_values[Black]),
  NUM_NONEMPTY_SMALL = ((NUM_NONEMPTY_SMALL_FIRST < NUM_NONEMPTY_SMALL_SECOND) ? NUM_NONEMPTY_SMALL_FIRST : NUM_NONEMPTY_SMALL_SECOND),
  SMALLENCODE_MAX =   (NUM_NONEMPTY_SMALL * (1 + 1 + MAX_BYTES_PER_SPEC))
                    + (MAX_NR_GHOSTS * (1 + 1 + MAX_BYTES_PER_SPEC))
                    + COMMONENCODE_MAX,
  LARGEENCODE_MAX =   nr_rows_on_board
                    + ((nr_rows_on_board * nr_files_on_board) * (1 + MAX_BYTES_PER_SPEC))
                    + (MAX_NR_GHOSTS * (1 + 1 + MAX_BYTES_PER_SPEC))
                    + COMMONENCODE_MAX,
  PROOFENCODE_MAX =   nr_rows_on_board
                    + ((nr_rows_on_board * nr_files_on_board) * 2)
                    + 1
                    + (MAX_NR_GHOSTS * (1 + 1 + MAX_BYTES_PER_SPEC))
                    + COMMONENCODE_MAX
};

enum
{
  MAX_LENGTH_OF_ENCODING = ((SMALLENCODE_MAX > LARGEENCODE_MAX) ? ((SMALLENCODE_MAX > PROOFENCODE_MAX) ? SMALLENCODE_MAX : PROOFENCODE_MAX)
                                                                : ((LARGEENCODE_MAX > PROOFENCODE_MAX) ? LARGEENCODE_MAX : PROOFENCODE_MAX)),
  hashbuf_length = (MAX_LENGTH_OF_ENCODING * sizeof(byte)) + offsetof(BCMemValue, Data)
};

typedef union
{
    BCMemValue cmv;
    byte buffer[hashbuf_length];
} HashBuffer;

extern HashBuffer hashBuffers[maxply+1];

#if defined(FXF)
extern unsigned long  hash_max_kilo_storable_positions;
#endif

/* exported functions */
void check_hash_assumptions(void);

/* Allocate memory for the hash table. If the requested amount of
 * memory isn't available, reduce the amount until allocation
 * succeeds.
 * @param nr_kilos number of kilo-bytes to allocate
 * @return number of kilo-bytes actually allocated
 */
unsigned long allochash(unsigned long nr_kilos);

/* Determine whether the hash table has been successfully allocated
 * @return true iff the hashtable has been allocated
 */
boolean is_hashtable_allocated(void);

void HashStats(unsigned int level, char const *trailer);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

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
void hash_opener_solve(slice_index si);

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
void attack_hashed_tester_solve(slice_index si);

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
void attack_hashed_solve(slice_index si);

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
void help_hashed_solve(slice_index si);

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
void help_hashed_tester_solve(slice_index si);

/* Instrument stipulation with hashing slices
 * @param si identifies slice where to start
 */
void solving_insert_hashing(slice_index si);

#endif
