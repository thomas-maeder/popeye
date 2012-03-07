/******************** MODIFICATIONS to pyhash.h **************************
**
** Date       Who  What
**
** 1994            Original
**
**************************** End of List ******************************/

#if !defined(PYHASH_H)
#define PYHASH_H

#include "DHT/dhtbcmem.h"
#include "py.h"
#include "stipulation/battle_play/attack_play.h"
#include "stipulation/help_play/play.h"

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

extern stip_length_type hashBufferValidity[maxply+1];

void validateHashBuffer(stip_length_type validity_value);

/* if (guard) invalidate hash buffer for ply nbply
 * (making guard a parameter prevents it from being evaluated
 * #if !defined(NDEBUG))
 * @param guard guard
 */
void invalidateHashBuffer(void);

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

/* Spin a tester slice off a STAttackHashedTester slice
 * @param base_slice identifies the STAttackHashedTester slice
 * @return id of allocated slice
 */
void spin_off_testers_attack_hashed(slice_index si, stip_structure_traversal *st);

/* Spin a tester slice off a STHelpHashed slice
 * @param base_slice identifies the STHelpHashed slice
 * @return id of allocated slice
 */
void spin_off_testers_help_hashed(slice_index si, stip_structure_traversal *st);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_hashed_tester_attack(slice_index si, stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type attack_hashed_attack(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type help_hashed_help(slice_index si, stip_length_type n);

/* Solve in a number of half-moves
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move leading to the current position has
 *                           turned out to be illegal
 *            n   solution found
 *            n+2 no solution found
 */
stip_length_type help_hashed_tester_help(slice_index si, stip_length_type n);

/* Instrument stipulation with hashing slices
 * @param si identifies slice where to start
 */
void stip_insert_hash_slices(slice_index si);

#endif
