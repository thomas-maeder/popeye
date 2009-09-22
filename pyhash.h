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
#include "pytable.h"
#include "pydirect.h"

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

extern boolean isHashBufferValid[maxply+1];

void validateHashBuffer(void);

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

void inithash (void);
void closehash (void);
void HashStats(unsigned int level, char *trailer);

void IncHashRateLevel(void);
void DecHashRateLevel(void);

/* Allocate a STDirectHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STDirectHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_directhashed_slice(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type direct_hashed_has_solution(slice_index si);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @return number of half moves effectively used
 *         n+2 if no continuation was found
 */
stip_length_type direct_hashed_solve_continuations_in_n(table continuations,
                                                        slice_index si,
                                                        stip_length_type n);

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type direct_hashed_solve_threats_in_n(table threats,
                                                  slice_index si,
                                                  stip_length_type n);

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_hashed_are_threats_refuted_in_n(table threats,
                                               stip_length_type len_threat,
                                               slice_index si,
                                               stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 * @return length of solution found, i.e.:
 *            0 defense put defender into self-check
 *            n_min..n length of shortest solution found
 *            >n no solution found
 *         (the second case includes the situation in self
 *         stipulations where the defense just played has reached the
 *         goal (in which case n_min<slack_length_direct and we return
 *         n_min)
 */
stip_length_type direct_hashed_has_solution_in_n(slice_index si,
                                                 stip_length_type n,
                                                 stip_length_type n_min);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_hashed_solve(slice_index si);

/* Allocate a STHelpHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STHelpHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_helphashed_slice(slice_index si);

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean hashed_help_solve(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_help_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_help_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_help_solve_continuations_in_n(table continuations,
                                          slice_index si,
                                          stip_length_type n);

/* Allocate a STSeriesHashed slice for a STBranch* slice and insert
 * it at the STBranch* slice's position. 
 * The STSeriesHashed takes the place of the STBranch* slice.
 * @param si identifies STBranch* slice
 */
void insert_serieshashed_slice(slice_index si);

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_series_solve_in_n(slice_index si, stip_length_type n);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_series_has_solution_in_n(slice_index si, stip_length_type n);

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_series_solve_threats_in_n(table threats,
                                      slice_index si,
                                      stip_length_type n);

#endif
