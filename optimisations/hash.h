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
#include "solving/ply.h"
#include <stddef.h>

#if defined(TESTHASH)
#       if !defined(HASHRATE)
#               define HASHRATE
#       endif   /* !HASHRATE */
#endif  /* TESTHASH */

/* typedefs */
typedef unsigned char byte;

enum
{
  MAX_LENGTH_OF_ENCODING = (((nr_rows_on_board * nr_files_on_board) + underworld_capacity) * 6) +
			                     18 + maxinum + nr_rows_on_board + (1 + maxply),
/* MAX_LENGTH_OF_ENCODING = (((nr_rows_on_board * nr_files_on_board) + underworld_capacity) * 6) +
	                          18 + maxinum + (en_passant_top[nbply] - en_passant_top[nbply-1]), */
  hashbuf_length = (MAX_LENGTH_OF_ENCODING * sizeof(byte)) + offsetof(BCMemValue, Data)
};

typedef union
{
    BCMemValue cmv;
    byte buffer[hashbuf_length];
} HashBuffer;

extern HashBuffer hashBuffers[maxply+1];

extern unsigned long  hash_max_number_storable_positions;

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
