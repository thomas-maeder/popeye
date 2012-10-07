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
#include "py.h"
#include "solving/solve.h"

/* typedefs */
typedef unsigned char byte;

enum
{
  hashbuf_length = 256
};

typedef union
{
    BCMemValue cmv;
    byte buffer[hashbuf_length];
} HashBuffer;

extern HashBuffer hashBuffers[maxply+1];

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

/* Initialise the hashing machinery for the current stipulation
 * @param si identifies the root slice of the stipulation
 */
void inithash (slice_index si);

/* Uninitialise the hashing machinery
 */
void closehash (void);

void HashStats(unsigned int level, char *trailer);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_hashed_tester_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_hashed_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type help_hashed_solve(slice_index si, stip_length_type n);

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type help_hashed_tester_solve(slice_index si, stip_length_type n);

/* Instrument stipulation with hashing slices
 * @param si identifies slice where to start
 */
void stip_insert_hash_slices(slice_index si);

#endif
