/********************* MODIFICATIONS to pyhash.c ***********************
 **
 ** Date       Who  What
 **
 ** 2003/05/12 TLi  hashing bug fixed: h= + intel did not find all solutions .
 **
 ** 2004/03/22 TLi  hashing for exact-* stipulations improved
 **
 ** 2005/02/01 TLi  function hashdefense is not used anymore...
 **
 ** 2005/02/01 TLi  in branch_d_does_attacker_win and invref exchanged the inquiry into the hash
 **                 table for "white can mate" and "white cannot mate" because
 **                 it is more likely that a position has no solution
 **                 This yields an incredible speedup of .5-1%  *fg*
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)  
 **
 ** 2009/01/03 SE   New condition: Disparate Chess (invented: R.Bedoni)  
 **
 **************************** End of List ******************************/

/**********************************************************************
 ** We hash.
 **
 ** SmallEncode and LargeEncode are functions to encode the current
 ** position. SmallEncode is used when the starting position contains
 ** less than or equal to eight pieces. LargeEncode is used when more
 ** pieces are present. The function TellSmallEncode and TellLargeEncode
 ** are used to decide what encoding to use. Which function to use is
 ** stored in encode.
 ** SmallEncode encodes for each piece the square where it is located.
 ** LargeEncode uses the old scheme introduced by Torsten: eight
 ** bytes at the beginning give in 64 bits the locations of the pieces
 ** coded after the eight bytes. Both functions give for each piece its
 ** type (1 byte) and specification (2 bytes). After this information
 ** about ep-captures, Duellants and Imitators are coded.
 **
 ** The hash table uses a dynamic hashing scheme which allows dynamic
 ** growth and shrinkage of the hashtable. See the relevant dht* files
 ** for more details. Two procedures are used:
 **   dhtLookupElement: This procedure delivers
 ** a nil pointer, when the given position is not in the hashtable,
 ** or a pointer to a hashelement.
 **   dhtEnterElement:  This procedure enters an encoded position
 ** with its values into the hashtable.
 **
 ** When there is no more memory, or more than MaxPositions positions
 ** are stored in the hash-table, then some positions are removed
 ** from the table. This is done in the compress procedure.
 ** This procedure uses a little improved scheme introduced by Torsten.
 ** The selection of positions to remove is based on the value of
 ** information gathered about this position. The information about
 ** a position "unsolvable in 2 moves" is less valuable than "unsolvable
 ** in 5 moves", since the former can be recomputed faster. For the other
 ** type of information ("solvable") the comparison is the other way round.
 ** The compression of the table is an expensive operation, in a lot
 ** of exeperiments it has shown to be quite effective in keeping the
 ** most valuable information, and speeds up the computation time
 ** considerably. But to be of any use, there must be enough memory to
 ** to store more than 800 positions.
 ** Now Torsten changed popeye so that all stipulations use hashing.
 ** There seems to be no real penalty in using hashing, even if the
 ** hit ratio is very small and only about 5%, it speeds up the
 ** computation time by 30%.
 ** I changed the output of hashstat, since its really informative
 ** to see the hit rate.
 **
 ** inithash()
 **   -- enters the startposition into the hash-table.
 **   -- determines which encode procedure to use
 **   -- Check's for the MaxPostion/MaxMemory settings
 **
 ** closehash()
 **   -- deletes the hashtable and gives back allocated storage.
 **
 ***********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* TurboC and BorlandC  TLi */
#if defined(__TURBOC__)
# include <mem.h>
# include <alloc.h>
# include <conio.h>
#else
# include <memory.h>
#endif  /* __TURBOC__ */

#include "py.h"
#include "pyproc.h"
#include "pydata.h"
#include "pymsg.h"
#include "pyhash.h"
#include "pyint.h"
#include "DHT/dhtvalue.h"
#include "DHT/dht.h"
#include "pyproof.h"
#include "pystip.h"
#include "pybrafrk.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "trace.h"

static struct dht *pyhash;

static char    piece_nbr[PieceCount];
static boolean one_byte_hash;
static unsigned int bytes_per_spec;
static unsigned int bytes_per_piece;

static boolean is_there_slice_with_nonstandard_min_length;

unsigned long int compression_counter;

HashBuffer hashBuffers[maxply+1];

boolean isHashBufferValid[maxply+1];

void validateHashBuffer(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceCurrentHashBuffer();

  isHashBufferValid[nbply] = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void invalidateHashBuffer(void)
{
  TraceFunctionEntry(__func__);

  TraceValue("%u\n",nbply);
  isHashBufferValid[nbply] = false;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

#if defined(TESTHASH)
#define ifTESTHASH(x)   x
#if defined(__unix)
#include <unistd.h>
static void *OldBreak;
extern int dhtDebug;
#endif /*__unix*/
#else
#define ifTESTHASH(x)
#endif /*TESTHASH*/

#if defined(HASHRATE)
#define ifHASHRATE(x)   x
static unsigned long use_pos, use_all;
#else
#define ifHASHRATE(x)
#endif /*HASHRATE*/

/* New Version for more ply's */
enum
{
  ByteMask = (1u<<CHAR_BIT)-1,
  BitsForPly = 10      /* Up to 1023 ply possible */
};

static void (*encode)(void);

typedef unsigned int data_type;

typedef struct
{
	dhtValue Key;
    data_type data;
} element_t;

static slice_index base_slice[max_nr_slices];

void hash_reset_derivations(void)
{
  slice_index si;
  for (si = 0; si!=max_nr_slices; ++si)
    base_slice[si] = no_slice;
}

void hash_slice_is_derived_from(slice_index derived, slice_index base)
{
  base_slice[derived] = base;
}

/* Hashing properties of stipulation slices
 */
typedef struct
{
    unsigned int size;
    unsigned int valueOffset;

    union
    {
        struct
        {
            unsigned int offsetSucc;
            unsigned int maskSucc;
            unsigned int offsetNoSucc;
            unsigned int maskNoSucc;
        } d;
        struct
        {
            unsigned int offsetNoSucc;
            unsigned int maskNoSucc;
        } h;
        struct
        {
            unsigned int offsetNoSucc;
            unsigned int maskNoSucc;
        } s;
    } u;
} slice_properties_t;

static slice_properties_t slice_properties[max_nr_slices];

static unsigned int bit_width(unsigned int value)
{
  unsigned int result = 0;
  while (value!=0)
  {
    ++result;
    value /= 2;
  }

  return result;
}

static boolean slice_property_offset_shifter(slice_index si,
                                             slice_traversal *st)
{
  boolean result;
  unsigned int const * const delta = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_properties[si].valueOffset -= *delta;

  TraceValue("%u",*delta);
  TraceValue("->%u\n",slice_properties[si].valueOffset);

  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_property_offset_shifters[] =
{
  &slice_property_offset_shifter, /* STBranchDirect */
  &slice_property_offset_shifter, /* STBranchDirectDefender */
  &slice_property_offset_shifter, /* STBranchHelp */
  &slice_property_offset_shifter, /* STBranchSeries */
  &slice_property_offset_shifter, /* STBranchFork */
  &slice_property_offset_shifter, /* STLeafDirect */
  &slice_property_offset_shifter, /* STLeafHelp */
  &slice_property_offset_shifter, /* STLeafSelf */
  &slice_property_offset_shifter, /* STLeafForced */
  &slice_property_offset_shifter, /* STReciprocal */
  &slice_property_offset_shifter, /* STQuodlibet */
  &slice_property_offset_shifter, /* STNot */
  &slice_property_offset_shifter, /* STMoveInverter */
  &slice_property_offset_shifter, /* STHelpRoot */
  &slice_property_offset_shifter, /* STHelpAdapter */
  &slice_property_offset_shifter, /* STHelpHashed */
  &slice_property_offset_shifter  /* STReflexGuard */
};

typedef struct
{
    unsigned int nrBitsLeft;
    unsigned int valueOffset;
} slice_initializer_state;

/* Initialise a slice_properties element representing direct play
 * @param si root slice of subtree
 * @param length number of attacker's moves of help slice
 * @param sis state of slice properties initialisation
 */
static void init_slice_property_direct(slice_index si,
                                       unsigned int length,
                                       slice_initializer_state *sis)
{
  unsigned int const size = bit_width(length);
  data_type const mask = (1<<size)-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",sis->nrBitsLeft);
  TraceFunctionParam("%u",sis->valueOffset);
  TraceFunctionParamListEnd();

  sis->valueOffset -= size;
  TraceValue("%u",size);
  TraceValue("->%u\n",sis->valueOffset);

  slice_properties[si].size = size;
  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u\n",slice_properties[si].valueOffset);

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.d.offsetNoSucc = sis->nrBitsLeft;
  slice_properties[si].u.d.maskNoSucc = mask << sis->nrBitsLeft;

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.d.offsetSucc = sis->nrBitsLeft;
  slice_properties[si].u.d.maskSucc = mask << sis->nrBitsLeft;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a slice_properties element representing help play
 * @param si root slice of subtree
 * @param length number of half moves of help slice
 * @param sis state of slice properties initialisation
 */
static void init_slice_property_help(slice_index si,
                                     unsigned int length,
                                     slice_initializer_state *sis)
{
  unsigned int const size = bit_width((length+1)/2);
  data_type const mask = (1<<size)-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_properties[si].size = size;
  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u",size);
  TraceValue("%u",mask);
  TraceValue("%u\n",slice_properties[si].valueOffset);

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.h.offsetNoSucc = sis->nrBitsLeft;
  slice_properties[si].u.h.maskNoSucc = mask << sis->nrBitsLeft;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Initialise a slice_properties element representing series play
 * @param si root slice of subtree
 * @param length number of half moves of series slice
 * @param sis state of slice properties initialisation
 */
static void init_slice_property_series(slice_index si,
                                       unsigned int length,
                                       slice_initializer_state *sis)
{
  unsigned int const size = bit_width(length);
  data_type const mask = (1<<size)-1;

  sis->valueOffset -= size;

  slice_properties[si].size = size;
  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u",si);
  TraceValue("%u\n",slice_properties[si].valueOffset);

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.s.offsetNoSucc = sis->nrBitsLeft;
  slice_properties[si].u.s.maskNoSucc = mask << sis->nrBitsLeft;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a help leaf
 * @param leaf root slice of subtree
 * @param st address of structure defining traversal
 * @return true
 */
static boolean init_slice_properties_leaf_help(slice_index leaf,
                                               slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  sis->valueOffset -= bit_width(1);
  init_slice_property_help(leaf,1,st->param);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a direct leaf
 * @param leaf root slice of subtree
 * @param st address of structure defining traversal
 * @return true
 */
static boolean init_slice_properties_leaf_direct(slice_index leaf,
                                                 slice_traversal *st)
{
  init_slice_property_direct(leaf,1,st->param);
  return true;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a forced leaf
 * @param leaf root slice of subtree
 * @param st address of structure defining traversal
 * @return true
 */
static boolean init_slice_properties_leaf_forced(slice_index leaf,
                                                 slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state const * const sis = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",leaf);
  TraceFunctionParamListEnd();

  slice_properties[leaf].valueOffset = sis->valueOffset;
  TraceValue("%u",leaf);
  TraceValue("%u\n",slice_properties[leaf].valueOffset);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a pipe for which we don't
 * have a more specialised function
 * @param leaf root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for pipe and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_pipe(slice_index pipe,
                                          slice_traversal *st)
{
  boolean result;
  slice_index const next = slices[pipe].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  result = traverse_slices(next,st);
  slice_properties[pipe].valueOffset = slice_properties[next].valueOffset;
  TraceValue("%u",pipe);
  TraceValue("%u\n",slice_properties[pipe].valueOffset);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a fork
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for pipe and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_fork(slice_index fork,
                                          slice_traversal *st)
{
  boolean result1;
  boolean result2;

  slice_initializer_state * const sis = st->param;

  unsigned int const save_valueOffset = sis->valueOffset;
      
  slice_index const op1 = slices[fork].u.fork.op1;
  slice_index const op2 = slices[fork].u.fork.op2;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  slice_properties[fork].valueOffset = sis->valueOffset;

  result1 = traverse_slices(op1,st);
  sis->valueOffset = save_valueOffset;
  result2 = traverse_slices(op2,st);

  TraceValue("%u",op1);
  TraceValue("%u",slice_properties[op1].valueOffset);
  TraceValue("%u",op2);
  TraceValue("%u\n",slice_properties[op2].valueOffset);

  /* both operand slices must have the same valueOffset, or the
   * shorter one will dominate the longer one */
  if (slice_properties[op1].valueOffset>slice_properties[op2].valueOffset)
  {
    unsigned int delta = (slice_properties[op1].valueOffset
                          -slice_properties[op2].valueOffset);
    slice_traversal st;
    slice_traversal_init(&st,&slice_property_offset_shifters,&delta);
    traverse_slices(op1,&st);
  }
  else if (slice_properties[op2].valueOffset>slice_properties[op1].valueOffset)
  {
    unsigned int delta = (slice_properties[op2].valueOffset
                          -slice_properties[op1].valueOffset);
    slice_traversal st;
    slice_traversal_init(&st,&slice_property_offset_shifters,&delta);
    traverse_slices(op2,&st);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u", result1 && result2);
  TraceFunctionResultEnd();
  return result1 && result2;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a direct branch
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for branch and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_branch_direct(slice_index branch,
                                                   slice_traversal *st)
{
  boolean result;
  slice_index const base = base_slice[branch];

  if (base==no_slice)
  {
    stip_length_type const length = slices[branch].u.pipe.u.branch.length;
    init_slice_property_direct(branch,length,st->param);
    result = slice_traverse_children(branch,st);
  }
  else
    result = true;

  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a direct branch defender
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for branch and its children have been
 *         successfully initialised
 */
static
boolean init_slice_properties_branch_direct_defender(slice_index branch,
                                                     slice_traversal *st)
{
  slice_index const
      tg = slices[branch].u.pipe.u.branch_d_defender.towards_goal;
  boolean const result = traverse_slices(tg,st);
  init_slice_properties_pipe(branch,st);
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for si and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_help_hashed(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;
  unsigned int const length = slices[si].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_slice_property_help(si,length-slack_length_help,sis);

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for si and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_help_adapter(slice_index si,
                                                  slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;
  slice_index branch1;
  slice_index const fork = slices[si].u.pipe.u.help_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  branch1 = branch_find_slice(STHelpHashed,si);
  if (branch1!=no_slice)
  {
    stip_length_type const length = slices[branch1].u.pipe.u.branch.length;
    unsigned int const width = bit_width((length-slack_length_help+1)/2);
    TraceValue("%u\n",width);
    sis->valueOffset -= width;
  }

  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u\n",slice_properties[si].valueOffset);

  slice_traverse_children(si,st);

  traverse_slices(slices[fork].u.pipe.u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a series branch
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for branch and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_branch_series(slice_index branch,
                                                   slice_traversal *st)
{
  boolean const result = true;
  stip_length_type const length = slices[branch].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  init_slice_property_series(branch,length-slack_length_series,st->param);

  slice_traverse_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a branch fork
 * @param branch_fork root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for branch_fork and its children
 *         have been successfully initialised
 */
static boolean init_slice_properties_branch_fork(slice_index branch_fork,
                                                 slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch_fork);
  TraceFunctionParamListEnd();

  init_slice_properties_pipe(branch_fork,st);

  /* not traversing towards goal - slice adapter has to do that */

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_properties_initalisers[] =
{
  &init_slice_properties_branch_direct,          /* STBranchDirect */
  &init_slice_properties_branch_direct_defender, /* STBranchDirectDefender */
  &init_slice_properties_pipe,                   /* STBranchHelp */
  &init_slice_properties_branch_series,          /* STBranchSeries */
  &init_slice_properties_branch_fork,            /* STBranchFork */
  &init_slice_properties_leaf_direct,            /* STLeafDirect */
  &init_slice_properties_leaf_help,              /* STLeafHelp */
  &init_slice_properties_leaf_direct,            /* STLeafSelf */
  &init_slice_properties_leaf_forced,            /* STLeafForced */
  &init_slice_properties_fork,                   /* STReciprocal */
  &init_slice_properties_fork,                   /* STQuodlibet */
  &init_slice_properties_pipe,                   /* STNot */
  &init_slice_properties_pipe,                   /* STMoveInverter */
  &init_slice_properties_help_adapter,           /* STHelpRoot */
  &init_slice_properties_help_adapter,           /* STHelpAdapter */
  &init_slice_properties_help_hashed,            /* STHelpHashed */
  &init_slice_properties_pipe                    /* STReflexGuard */
};

/* Callback for traverse_slices() that copies slice_properties from
 * base to derived slice.
 * @param si identifies potentially derived slice
 * @param dummy - necessary to conform to callback interface
 * @return true iff the properties of all the slices that require it
 *         have been successfully inherited
 */
static boolean inherit_slice_properties_from_base(slice_index si,
                                                  slice_traversal *st)
{
  boolean result;
  slice_index const base = base_slice[si];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (base!=no_slice)
    slice_properties[si] = slice_properties[base];

  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_properties_inheriters[] =
{
  &inherit_slice_properties_from_base,  /* STBranchDirect */
  &slice_traverse_children,             /* STBranchDirectDefender */
  &slice_traverse_children,             /* STBranchHelp */
  &slice_traverse_children,             /* STBranchSeries */
  &slice_traverse_children,             /* STBranchFork */
  &slice_traverse_children,             /* STLeafDirect */
  &slice_traverse_children,             /* STLeafHelp */
  &slice_traverse_children,             /* STLeafSelf */
  &slice_traverse_children,             /* STLeafForced */
  &slice_traverse_children,             /* STReciprocal */
  &slice_traverse_children,             /* STQuodlibet */
  &slice_traverse_children,             /* STNot */
  &slice_traverse_children,             /* STMoveInverter */
  &slice_traverse_children,             /* STHelpRoot */
  &slice_traverse_children,             /* STHelpAdapter */
  &slice_traverse_children,             /* STHelpHashed */
  &slice_traverse_children              /* STReflexGuard */
};

/* Find out whether a branch has a non-standard length (i.e. is exact)
 * @param branch identifies branch
 */
static boolean  non_standard_length_finder_branch_direct(slice_index branch,
                                                         slice_traversal *st)
{
  boolean const result = true;
  stip_length_type const length = slices[branch].u.pipe.u.branch.length;
  boolean * const nonstandard = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  if (slices[branch].u.pipe.u.branch.min_length==length
      && length>slack_length_direct+1)
    *nonstandard = true;

  slice_traverse_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find out whether a branch has a non-standard length (i.e. is exact)
 * @param branch identifies branch
 */
static boolean non_standard_length_finder_help_branch(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;
  boolean * const nonstandard = st->param;
  stip_length_type const length = slices[si].u.pipe.u.help_adapter.length;
  slice_index const fork = slices[si].u.pipe.u.help_adapter.fork;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.pipe.u.help_adapter.min_length==length
      && length>slack_length_help+1)
    *nonstandard = true;

  traverse_slices(slices[fork].u.pipe.u.branch_fork.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find out whether a branch has a non-standard length (i.e. is exact)
 * @param branch identifies branch
 */
static boolean non_standard_length_finder_branch_series(slice_index branch,
                                                        slice_traversal *st)
{
  boolean const result = true;
  stip_length_type const length = slices[branch].u.pipe.u.branch.length;
  boolean * const nonstandard = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",branch);
  TraceFunctionParamListEnd();

  if (slices[branch].u.pipe.u.branch.min_length==length
      && length>slack_length_series+1)
    *nonstandard = true;

  slice_traverse_children(branch,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const non_standard_length_finders[] =
{
  &non_standard_length_finder_branch_direct, /* STBranchDirect */
  &slice_traverse_children,                  /* STBranchDirectDefender */
  &slice_traverse_children,                  /* STBranchHelp */
  &non_standard_length_finder_branch_series, /* STBranchSeries */
  &slice_traverse_children,                  /* STBranchFork */
  &slice_traverse_children,                  /* STLeafDirect */
  &slice_traverse_children,                  /* STLeafHelp */
  &slice_traverse_children,                  /* STLeafSelf */
  &slice_traverse_children,                  /* STLeafForced */
  &slice_traverse_children,                  /* STReciprocal */
  &slice_traverse_children,                  /* STQuodlibet */
  &slice_traverse_children,                  /* STNot */
  &slice_traverse_children,                  /* STMoveInverter */
  &slice_traverse_children,                  /* STHelpRoot */
  &non_standard_length_finder_help_branch,   /* STHelpAdapter */
  &non_standard_length_finder_help_branch,   /* STHelpHashed */
  &slice_traverse_children                   /* STReflexGuard */
};

static boolean findMinimalValueOffset(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  unsigned int * const minimalValueOffset = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",slice_properties[si].valueOffset);
  if (*minimalValueOffset>slice_properties[si].valueOffset)
    *minimalValueOffset = slice_properties[si].valueOffset;

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const min_valueOffset_finders[] =
{
  &findMinimalValueOffset,  /* STBranchDirect */
  &findMinimalValueOffset,  /* STBranchDirectDefender */
  &slice_traverse_children, /* STBranchHelp */
  &findMinimalValueOffset,  /* STBranchSeries */
  &findMinimalValueOffset,  /* STBranchFork */
  &findMinimalValueOffset,  /* STLeafDirect */
  &findMinimalValueOffset,  /* STLeafHelp */
  &findMinimalValueOffset,  /* STLeafSelf */
  &findMinimalValueOffset,  /* STLeafForced */
  &findMinimalValueOffset,  /* STReciprocal */
  &findMinimalValueOffset,  /* STQuodlibet */
  &findMinimalValueOffset,  /* STNot */
  &findMinimalValueOffset,  /* STMoveInverter */
  &findMinimalValueOffset,  /* STHelpRoot */
  &findMinimalValueOffset,  /* STHelpAdapter */
  &findMinimalValueOffset,  /* STHelpHashed */
  &findMinimalValueOffset   /* STReflexGuard */
};

static boolean reduceValueOffset(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  unsigned int const * const minimalValueOffset = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_properties[si].valueOffset -= *minimalValueOffset;
  TraceValue("%u\n",slice_properties[si].valueOffset);

  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const valueOffset_reducers[] =
{
  &reduceValueOffset,       /* STBranchDirect */
  &reduceValueOffset,       /* STBranchDirectDefender */
  &slice_traverse_children, /* STBranchHelp */
  &reduceValueOffset,       /* STBranchSeries */
  &reduceValueOffset,       /* STBranchFork */
  &reduceValueOffset,       /* STLeafDirect */
  &reduceValueOffset,       /* STLeafHelp */
  &reduceValueOffset,       /* STLeafSelf */
  &reduceValueOffset,       /* STLeafForced */
  &reduceValueOffset,       /* STReciprocal */
  &reduceValueOffset,       /* STQuodlibet */
  &reduceValueOffset,       /* STNot */
  &reduceValueOffset,       /* STMoveInverter */
  &reduceValueOffset,       /* STHelpRoot */
  &reduceValueOffset,       /* STHelpAdapter */
  &reduceValueOffset,       /* STHelpHashed */
  &reduceValueOffset        /* STReflexGuard */
};

/* Initialise the slice_properties array according to the current
 * stipulation slices.
 */
static void init_slice_properties(void)
{
  slice_traversal st;
  slice_initializer_state sis = {
    sizeof(data_type)*CHAR_BIT,
    sizeof(data_type)*CHAR_BIT
  };

  unsigned int minimalValueOffset = sizeof(data_type)*CHAR_BIT;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&slice_properties_initalisers,&sis);
  traverse_slices(root_slice,&st);

  slice_traversal_init(&st,&slice_properties_inheriters,0);
  traverse_slices(root_slice,&st);
  
  is_there_slice_with_nonstandard_min_length = false;
  slice_traversal_init(&st,
                       &non_standard_length_finders,
                       &is_there_slice_with_nonstandard_min_length);
  traverse_slices(root_slice,&st);

  slice_traversal_init(&st,&min_valueOffset_finders,&minimalValueOffset);
  traverse_slices(root_slice,&st);

  TraceValue("%u\n",minimalValueOffset);

  slice_traversal_init(&st,&valueOffset_reducers,&minimalValueOffset);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Pseudo hash table element - template for fast initialization of
 * newly created actual table elements
 */
static dhtElement template_element;


static void set_value_direct_nosucc(dhtElement *he,
                                    slice_index si,
                                    hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();
  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TracePointerValue("%p ",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void set_value_direct_succ(dhtElement *he,
                                  slice_index si,
                                  hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();
  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TracePointerValue("%p ",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void set_value_help(dhtElement *he, slice_index si, hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.h.maskNoSucc;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();
  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void set_value_series(dhtElement *he,
                             slice_index si,
                             hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.s.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.s.maskNoSucc;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();
  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static hash_value_type get_value_direct_succ(dhtElement const *he,
                                             slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TracePointerValue("%p ",&e->data);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static hash_value_type get_value_direct_nosucc(dhtElement const *he,
                                               slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TracePointerValue("%p ",&e->data);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static hash_value_type get_value_help(dhtElement const *he, slice_index si)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSucc;
  unsigned int const  mask = slice_properties[si].u.h.maskNoSucc;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static hash_value_type get_value_series(dhtElement const *he,
                                        slice_index si)
{
  unsigned int const offset = slice_properties[si].u.s.offsetNoSucc;
  unsigned int const mask = slice_properties[si].u.s.maskNoSucc;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of a direct slice (or leaf slice with
 * direct end) to the value of a hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of slice
 * @param length length of slice
 * @return value of contribution of slice si to *he's value
 */
static hash_value_type own_value_of_data_direct(dhtElement const *he,
                                                slice_index si,
                                                stip_length_type length)
{
  hash_value_type result;

  hash_value_type const succ = get_value_direct_succ(he,si);
  hash_value_type const nosucc = get_value_direct_nosucc(he,si);
  hash_value_type const succ_neg = length-succ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParamListEnd();

  TraceValue("%u",succ);
  TraceValue("%u\n",nosucc);

  assert(succ<=length);
  result = succ_neg>nosucc ? succ_neg : nosucc;
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of a help slice (or leaf slice with help
 * end) to the value of a hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of help slice
 * @return value of contribution of slice si to *he's value
 */
static hash_value_type own_value_of_data_help(dhtElement const *he,
                                              slice_index si)
{
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = get_value_help(he,si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of a series slice to the value of
 * a hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of series slice
 * @return value of contribution of slice si to *he's value
 */
static hash_value_type own_value_of_data_series(dhtElement const *he,
                                                slice_index si)
{
  return get_value_series(he,si);
}

/* Determine the contribution of a leaf slice to the value of
 * a hash table element node.
 * @param he address of hash table element to determine value of
 * @param leaf slice index of composite slice
 * @return value of contribution of the leaf slice to *he's value
 */
static hash_value_type own_value_of_data_leaf(dhtElement const *he,
                                              slice_index leaf)
{
  switch (slices[leaf].type)
  {
    case STLeafHelp:
      return own_value_of_data_help(he,leaf);

    case STLeafDirect:
    case STLeafSelf:
      return own_value_of_data_direct(he,leaf,1);

    case STLeafForced:
      return 0;

    default:
      assert(0);
      return 0;
  }
}

/* Determine the contribution of a composite slice to the value of
 * a hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of composite slice
 * @return value of contribution of the slice si to *he's value
 */
static hash_value_type own_value_of_data_composite(dhtElement const *he,
                                                   slice_index si)
{
  hash_value_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p ",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  switch (slices[si].type)
  {
    case STBranchDirect:
      result = own_value_of_data_direct(he,si,slices[si].u.pipe.u.branch.length);
      break;

    case STHelpHashed:
      result = own_value_of_data_help(he,si);
      break;

    case STBranchSeries:
      result = own_value_of_data_series(he,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of a stipulation subtree to the value of
 * a hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of subtree root slice
 * @return value of contribuation of the subtree to *he's value
 */
static hash_value_type value_of_data_recursive(dhtElement const *he,
                                               slice_index si)
{
  hash_value_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p ",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (base_slice[si]==no_slice)
  {
    unsigned int const offset = slice_properties[si].valueOffset;
    TraceValue("%u",slices[si].type);
    TraceValue("%u\n",slice_properties[si].valueOffset);

    switch (slices[si].type)
    {
      case STLeafSelf:
      case STLeafDirect:
      case STLeafHelp:
      {
        result = own_value_of_data_leaf(he,si) << offset;
        break;
      }

      case STLeafForced:
        result = 0;
        break;

      case STQuodlibet:
      case STReciprocal:
      {
        slice_index const op1 = slices[si].u.fork.op1;
        slice_index const op2 = slices[si].u.fork.op2;

        hash_value_type const nested_value1 = value_of_data_recursive(he,op1);
        hash_value_type const nested_value2 = value_of_data_recursive(he,op2);

        result = nested_value1>nested_value2 ? nested_value1 : nested_value2;
        break;
      }

      case STNot:
      case STMoveInverter:
      case STHelpRoot:
      case STBranchHelp:
      {
        slice_index const next = slices[si].u.pipe.next;
        result = value_of_data_recursive(he,next);
        break;
      }

      case STBranchDirect:
      {
        hash_value_type const own_value = own_value_of_data_composite(he,si);
        slice_index const peer = slices[si].u.pipe.next;
        hash_value_type const nested_value = value_of_data_recursive(he,peer);
        result = (own_value << offset) + nested_value;
        break;
      }

      case STBranchDirectDefender:
      {
        slice_index const next = slices[si].u.pipe.u.branch_d_defender.towards_goal;
        result = value_of_data_recursive(he,next);
        break;
      }

      case STBranchSeries:
      {
        hash_value_type const own_value = own_value_of_data_composite(he,si);

        slice_index const next = slices[si].u.pipe.next;
        hash_value_type const nested_value = value_of_data_recursive(he,next);
        TraceValue("%x ",own_value);
        TraceValue("%x\n",nested_value);

        result = (own_value << offset) + nested_value;
        break;
      }

      case STBranchFork:
      {
        slice_index const next = slices[si].u.pipe.next;
        slice_index const
            to_goal = slices[si].u.pipe.u.branch_fork.towards_goal;
        hash_value_type const next_value = value_of_data_recursive(he,next);
        hash_value_type const nested_value = value_of_data_recursive(he,
                                                                     to_goal);
        result = next_value+nested_value;
        break;
      }

      case STHelpHashed:
      {
        result = own_value_of_data_composite(he,si) << offset;
        break;
      }

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x",result);
  TraceFunctionResultEnd();
  return result;
}

/* How much is element *he worth to us? This information is used to
 * determine which elements to discard from the hash table if it has
 * reached its capacity.
 * @param he address of hash table element to determine value of
 * @return value of *he
 */
static hash_value_type value_of_data(dhtElement const *he)
{
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParamListEnd();

  result = value_of_data_recursive(he,root_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned long totalRemoveCount = 0;

static void compresshash (void)
{
  dhtElement *he;
  hash_value_type x;
  unsigned long RemoveCnt, ToDelete;
#if defined(TESTHASH)
  unsigned long initCnt, visitCnt, runCnt;
#endif

  ++compression_counter;
  he= dhtGetFirstElement(pyhash);
  if (he!=0)
  {
    hash_value_type min_val = value_of_data(he);
    he = dhtGetNextElement(pyhash);
    while (he)
    {
      x = value_of_data(he);
      if (x<min_val)
      {
        min_val = x;
        if (x==0)
          break;
      }
      he= dhtGetNextElement(pyhash);
    }

    RemoveCnt= 0;
    ToDelete= dhtKeyCount(pyhash)/16 + 1;
    if (ToDelete >= dhtKeyCount(pyhash))
      ToDelete= dhtKeyCount(pyhash);
    /* this is a pathological case: it may only occur, when we are so
     * low on memory, that only one or no position can be stored.
     */

#if defined(TESTHASH)
    printf("\nmin_val: %08x\n", min_val);
    printf("ToDelete: %ld\n", ToDelete);
    fflush(stdout);
    initCnt= dhtKeyCount(pyhash);
    runCnt= 0;
#endif  /* TESTHASH */

    while (RemoveCnt < ToDelete)
    {
      ++min_val;

#if defined(TESTHASH)
      printf("min_val: %08x\n", min_val);
      printf("RemoveCnt: %ld\n", RemoveCnt);
      fflush(stdout);
      visitCnt= 0;
#endif  /* TESTHASH */

      for (he = dhtGetFirstElement(pyhash);
           he!=0;
           he= dhtGetNextElement(pyhash))
        if (value_of_data(he)<min_val)
        {
          RemoveCnt++;
          totalRemoveCount++;
          dhtRemoveElement(pyhash, he->Key);
#if defined(TESTHASH)
          if (RemoveCnt + dhtKeyCount(pyhash) != initCnt)
          {
            fprintf(stdout,
                    "dhtRemove failed on %ld-th element of run %ld. "
                    "This was the %ld-th call to dhtRemoveElement.\n"
                    "RemoveCnt=%ld, dhtKeyCount=%ld, initCnt=%ld\n",
                    visitCnt, runCnt, totalRemoveCount,
                    RemoveCnt, dhtKeyCount(pyhash), initCnt);
            exit(1);
          }
#endif  /* TESTHASH */
        }
#if defined(TESTHASH)
      visitCnt++;
#endif  /* TESTHASH */
#if defined(TESTHASH)
      runCnt++;
      printf("run=%ld, RemoveCnt: %ld, missed: %ld\n",
             runCnt, RemoveCnt, initCnt-visitCnt);
      {
        int l, counter[16];
        int KeyCount=dhtKeyCount(pyhash);
        dhtBucketStat(pyhash, counter, 16);
        for (l=0; l< 16-1; l++)
          fprintf(stdout, "%d %d %d\n", KeyCount, l+1, counter[l]);
        printf("%d %d %d\n\n", KeyCount, l+1, counter[l]);
        if (runCnt > 9)
          printf("runCnt > 9 after %ld-th call to  dhtRemoveElement\n",
                 totalRemoveCount);
        dhtDebug= runCnt == 9;
      }
      fflush(stdout);
#endif  /* TESTHASH */

    }
#if defined(TESTHASH)
    printf("%ld;", dhtKeyCount(pyhash));
#if defined(HASHRATE)
    printf(" usage: %ld", use_pos);
    printf(" / %ld", use_all);
    printf(" = %ld%%", (100 * use_pos) / use_all);
#endif
#if defined(FREEMAP) && defined(FXF)
    PrintFreeMap(stdout);
#endif /*FREEMAP*/
#if defined(__TURBOC__)
    gotoxy(1, wherey());
#else
    printf("\n");
#endif /*__TURBOC__*/
#if defined(FXF)
    printf("\n after compression:\n");
    fxfInfo(stdout);
#endif /*FXF*/
#endif /*TESTHASH*/
  }
} /* compresshash */

#if defined(HASHRATE)
/* Level = 0: No output of HashStat
 * Level = 1: Output with every trace output
 * Level = 2: Output at each table compression
 * Level = 3: Output at every 1000th hash entry
 * a call to HashStats with a value of 0 will
 * always print
 */
static unsigned int HashRateLevel = 0;

void IncHashRateLevel(void)
{
  ++HashRateLevel;
  StdString("  ");
  PrintTime();
  logIntArg(HashRateLevel);
  Message(IncrementHashRateLevel);
  HashStats(0, "\n");
}

void DecHashRateLevel(void)
{
  if (HashRateLevel>0)
    --HashRateLevel;
  StdString("  ");
  PrintTime();
  logIntArg(HashRateLevel);
  Message(DecrementHashRateLevel);
  HashStats(0, "\n");
}

#else

void IncHashRateLevel(void)
{
  /* intentionally nothing */
}

void DecHashRateLevel(void)
{
  /* intentionally nothing */
}

#endif

void HashStats(unsigned int level, char *trailer)
{
#if defined(HASHRATE)
  int pos=dhtKeyCount(pyhash);
  char rate[60];

  if (level<=HashRateLevel)
  {
    StdString("  ");
    pos= dhtKeyCount(pyhash);
    logIntArg(pos);
    Message(HashedPositions);
    if (use_all > 0)
    {
      if (use_all < 10000)
        sprintf(rate, " %ld/%ld = %ld%%",
                use_pos, use_all, (use_pos*100) / use_all);
      else
        sprintf(rate, " %ld/%ld = %ld%%",
                use_pos, use_all, use_pos / (use_all/100));
    }
    else
      sprintf(rate, " -");
    StdString(rate);
    if (HashRateLevel > 3)
    {
      unsigned long msec;
      unsigned long Seconds;
      StopTimer(&Seconds,&msec);
      if (Seconds > 0)
      {
        sprintf(rate, ", %lu pos/s", use_all/Seconds);
        StdString(rate);
      }
    }
    if (trailer)
      StdString(trailer);
  }
#endif /*HASHRATE*/
}

static int estimateNumberOfHoles(slice_index si)
{
  int result = 0;

  /*
   * I assume an average of (nr_files_on_board*nr_rows_on_board -
   * number of pieces)/2 additional holes per position.
   */
  switch (slices[si].type)
  {
    case STBranchDirect:
      result = 2*slices[si].u.pipe.u.branch.length;
      break;

    case STBranchDirectDefender:
      result = 2*slices[si].u.pipe.u.branch_d_defender.length;
      break;

    case STBranchSeries:
      /* That's far too much. In a ser-h#5 there won't be more
       * than 5 holes in hashed positions.      TLi
       */
      result = slices[si].u.pipe.u.branch.length;
      break;

    case STBranchHelp:
    case STHelpRoot:
    case STBranchFork:
    case STMoveInverter:
    case STNot:
      result = estimateNumberOfHoles(slices[si].u.pipe.next);
      break;

    case STQuodlibet:
    case STReciprocal:
    {
      int const result1 = estimateNumberOfHoles(slices[si].u.fork.op1);
      int const result2 = estimateNumberOfHoles(slices[si].u.fork.op2);
      result = result1>result2 ? result1 : result2;
      break;
    }

    case STHelpHashed:
      result = 2*slices[si].u.pipe.u.branch.length;
      break;

    default:
      assert(0);
      break;
  }

  return result;
}

static int TellCommonEncodePosLeng(int len, int nbr_p)
{
  len++; /* Castling_Flag */

  if (CondFlag[haanerchess])
  {
    int nbr_holes = estimateNumberOfHoles(root_slice);
    if (nbr_holes > (nr_files_on_board*nr_rows_on_board-nbr_p)/2)
      nbr_holes= (nr_files_on_board*nr_rows_on_board-nbr_p)/2;
    len += bytes_per_piece*nbr_holes;
  }

  if (CondFlag[messigny])
    len+= 2;

  if (CondFlag[duellist])
    len+= 2;

  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue])
    len++;

  if (flag_synchron)
    len++;

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<inum[nbply]; imi_idx++)
      len++;

    /* coding of no. of imitators and average of one
       imitator-promotion assumed.
    */
    len+=2;
  }

  if (CondFlag[parrain])
    /*
    ** only one out of three positions with a capture
    ** assumed.
    */
    len++;

  if (OptFlag[nontrivial])
    len++;

  if (is_there_slice_with_nonstandard_min_length)
    len++;

  if (CondFlag[disparate])
    len++;

  return len;
} /* TellCommonEncodePosLeng */

static int TellLargeEncodePosLeng(void)
{
  square const *bnp;
  int       nbr_p= 0, len= 8;

  for (bnp= boardnum; *bnp; bnp++)
    if (e[*bnp] != vide)
    {
      len += bytes_per_piece;
      nbr_p++;  /* count no. of pieces and holes */
    }

  if (CondFlag[BGL])
    len+= sizeof BGL_white + sizeof BGL_black;

  len += nr_ghosts*bytes_per_piece;

  return TellCommonEncodePosLeng(len, nbr_p);
} /* TellLargeEncodePosLeng */

static int TellSmallEncodePosLeng(void)
{
  square const *bnp;
  int nbr_p= 0, len= 0;

  for (bnp= boardnum; *bnp; bnp++)
  {
    /* piece    p;
    ** Flags    pspec;
    */
    if (e[*bnp] != vide)
    {
      len += 1 + bytes_per_piece;
      nbr_p++;            /* count no. of pieces and holes */
    }
  }

  len += nr_ghosts*bytes_per_piece;
  
  return TellCommonEncodePosLeng(len, nbr_p);
} /* TellSmallEncodePosLeng */

static byte *CommonEncode(byte *bp)
{
  if (CondFlag[messigny]) {
    if (move_generation_stack[nbcou].capture == messigny_exchange) {
      *bp++ = (byte)(move_generation_stack[nbcou].arrival - square_a1);
      *bp++ = (byte)(move_generation_stack[nbcou].departure - square_a1);
    }
    else {
      *bp++ = (byte)(0);
      *bp++ = (byte)(0);
    }
  }
  if (CondFlag[duellist]) {
    *bp++ = (byte)(whduell[nbply] - square_a1);
    *bp++ = (byte)(blduell[nbply] - square_a1);
  }

  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue])
    *bp++ = (byte)(move_generation_stack[nbcou].departure - square_a1);

  if (flag_synchron)
    *bp++= (byte)(sq_num[move_generation_stack[nbcou].departure]
                  -sq_num[move_generation_stack[nbcou].arrival]
                  +64);

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;

    /* The number of imitators has to be coded too to avoid
     * ambiguities.
     */
    *bp++ = (byte)inum[nbply];
    for (imi_idx = 0; imi_idx<inum[nbply]; imi_idx++)
      *bp++ = (byte)(isquare[imi_idx]-square_a1);
  }

  if (OptFlag[nontrivial])
    *bp++ = (byte)(max_nr_nontrivial);

  if (CondFlag[parrain]) {
    /* a piece has been captured and can be reborn */
    *bp++ = (byte)(move_generation_stack[nbcou].capture - square_a1);
    if (one_byte_hash) {
      *bp++ = (byte)(pprispec[nbply])
          + ((byte)(piece_nbr[abs(pprise[nbply])]) << (CHAR_BIT/2));
    }
    else {
      *bp++ = pprise[nbply];
      *bp++ = (byte)(pprispec[nbply]>>CHAR_BIT);
      *bp++ = (byte)(pprispec[nbply]&ByteMask);
    }
  }

  if (is_there_slice_with_nonstandard_min_length)
    *bp++ = (byte)(nbply);

  if (ep[nbply]!=initsquare)
    *bp++ = (byte)(ep[nbply] - square_a1);

  *bp++ = castling_flag[nbply];     /* Castling_Flag */

  if (CondFlag[BGL]) {
    memcpy(bp, &BGL_white, sizeof BGL_white);
    bp += sizeof BGL_white;
    memcpy(bp, &BGL_black, sizeof BGL_black);
    bp += sizeof BGL_black;
  }

  if (CondFlag[disparate]) {
    *bp++ = (byte)(nbply>=2?pjoue[nbply]:vide);
  }

  return bp;
} /* CommonEncode */

static byte *LargeEncodePiece(byte *bp, byte *position,
                              int row, int col,
                              piece p, Flags pspec)
{
  if (!TSTFLAG(pspec, Neutral))
    SETFLAG(pspec, (p < vide ? Black : White));
  p = abs(p);
  if (one_byte_hash)
    *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
  else
  {
    unsigned int i;
    *bp++ = p;
    for (i = 0; i<bytes_per_spec; i++)
      *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
  }

  position[row] |= BIT(col);

  return bp;
}

static void LargeEncode(void)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *position = hb->cmv.Data;
  byte *bp = position+nr_rows_on_board;
  int row, col;
  square a_square = square_a1;
  ghost_index_type gi;

  /* detect cases where we encode the same position twice */
  assert(!isHashBufferValid[nbply]);

  /* clear the bits for storing the position of pieces */
  memset(position,0,nr_rows_on_board);

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow)
  {
    square curr_square = a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right)
    {
      piece const p = e[curr_square];
      if (p!=vide)
        bp = LargeEncodePiece(bp,position,row,col,p,spec[curr_square]);
    }
  }

  for (gi = 0; gi<nr_ghosts; ++gi)
  {
    square s = (ghosts[gi].ghost_square
                - nr_of_slack_rows_below_board*onerow
                - nr_of_slack_files_left_of_board);
    row = s/onerow;
    col = s%onerow;
    bp = LargeEncodePiece(bp,position,
                          row,col,
                          ghosts[gi].ghost_piece,ghosts[gi].ghost_flags);
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);

  validateHashBuffer();
} /* LargeEncode */

static byte *SmallEncodePiece(byte *bp,
                              int row, int col,
                              piece p, Flags pspec)
{
  if (!TSTFLAG(pspec,Neutral))
    SETFLAG(pspec, (p < vide ? Black : White));
  p= abs(p);
  *bp++= (byte)((row<<(CHAR_BIT/2))+col);
  if (one_byte_hash)
    *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
  else
  {
    unsigned int i;
    *bp++ = p;
    for (i = 0; i<bytes_per_spec; i++)
      *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
  }

  return bp;
}

static void SmallEncode(void)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *bp = hb->cmv.Data;
  square a_square = square_a1;
  int row;
  int col;
  ghost_index_type gi;

  /* detect cases where we encode the same position twice */
  assert(!isHashBufferValid[nbply]);

  for (row=0; row<nr_rows_on_board; row++, a_square += onerow)
  {
    square curr_square= a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square += dir_right)
    {
      piece const p = e[curr_square];
      if (p!=vide)
        bp = SmallEncodePiece(bp,row,col,p,spec[curr_square]);
    }
  }

  for (gi = 0; gi<nr_ghosts; ++gi)
  {
    square s = (ghosts[gi].ghost_square
                - nr_of_slack_rows_below_board*onerow
                - nr_of_slack_files_left_of_board);
    row = s/onerow;
    col = s%onerow;
    bp = SmallEncodePiece(bp,
                          row,col,
                          ghosts[gi].ghost_piece,ghosts[gi].ghost_flags);
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);

  validateHashBuffer();
}

boolean inhash(slice_index si, hashwhat what, hash_value_type val)
{
  boolean result = false;
  HashBuffer *hb = &hashBuffers[nbply];
  dhtElement const *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",what);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);

  if (!isHashBufferValid[nbply])
    (*encode)();

  ifHASHRATE(use_all++);

  /* TODO create hash slice(s) that are only active if we can allocate
   * the hash table. */
  he = pyhash==0 ? dhtNilElement : dhtLookupElement(pyhash, (dhtValue)hb);
  if (he==dhtNilElement)
    result = false;
  else
    switch (slices[si].type)
    {
      case STLeafDirect:
      case STLeafSelf:
        if (what==DirNoSucc)
        {
          hash_value_type const nosucc = get_value_direct_nosucc(he,si);
          assert(val==1);
          TraceValue("%u",slices[si].u.pipe.u.branch.length);
          TraceValue("%u\n",slices[si].u.pipe.u.branch.min_length);
          if (nosucc>=1)
          {
            ifHASHRATE(use_pos++);
            result = true;
          } else
            result = false;
        }
        else
        {
          hash_value_type const succ = get_value_direct_succ(he,si);
          assert(val==0);
          if (succ==0)
          {
            ifHASHRATE(use_pos++);
            result = true;
          } else
            result = false;
        }
        break;

      case STBranchDirect:
        if (what==DirNoSucc)
        {
          hash_value_type const nosucc = get_value_direct_nosucc(he,si);
          TraceValue("%u",slices[si].u.pipe.u.branch.length);
          TraceValue("%u\n",slices[si].u.pipe.u.branch.min_length);
          if (nosucc>=val
              && (nosucc+slices[si].u.pipe.u.branch.min_length
                  <=val+slices[si].u.pipe.u.branch.length))
          {
            ifHASHRATE(use_pos++);
            result = true;
          } else
            result = false;
        }
        else
        {
          hash_value_type const succ = get_value_direct_succ(he,si);
          if (succ<=val
              && (succ+slices[si].u.pipe.u.branch.length
                  >=val+slices[si].u.pipe.u.branch.min_length))
          {
            ifHASHRATE(use_pos++);
            result = true;
          } else
            result = false;
        }
        break;

      case STBranchSeries:
      {
        hash_value_type const nosucc = get_value_series(he,si);
        if (nosucc>=val
            && (nosucc+slices[si].u.pipe.u.branch.min_length
                <=val+slices[si].u.pipe.u.branch.length))
        {
          ifHASHRATE(use_pos++);
          result = true;
        }
        else
          result = false;
        break;
      }

      case STHelpHashed:
        {
          hash_value_type const nosucc = get_value_help(he,si);
          if (nosucc>=val
              && (nosucc+slices[si].u.pipe.u.branch.min_length
                  <=val+slices[si].u.pipe.u.branch.length))
          {
            ifHASHRATE(use_pos++);
            result = true;
          }
          else
            result = false;
        }
        break;

      case STLeafHelp:
        {
          hash_value_type const nosucc = get_value_help(he,si);
          assert(what==hash_help_insufficient_nr_half_moves);
          assert(val==1);
          if (nosucc>=1)
          {
            ifHASHRATE(use_pos++);
            result = true;
          }
          else
            result = false;
        }
        break;

      default:
        assert(0);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result; /* avoid compiler warning */
} /* inhash */

/* Initialise the bits representing a direct slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_direct(dhtElement *he,
                                slice_index si,
                                unsigned int length)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParamListEnd();

  set_value_direct_nosucc(he,si,0);
  set_value_direct_succ(he,si,length);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the bits representing a help slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_help(dhtElement *he, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  set_value_help(he,si,0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the bits representing a series slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_series(dhtElement *he, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",he);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  set_value_series(he,si,0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a slice while initialising a hash table element
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return true
 */
boolean init_element_leaf_h(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  dhtElement * const he = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_element_help(he,si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while initialising a hash table element
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return true
 */
boolean init_element_leaf_d(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  dhtElement * const he = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_element_direct(he,si,1);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while initialising a hash table element
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean init_element_branch_d(slice_index si, slice_traversal *st)
{
  boolean result;
  dhtElement * const he = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (base_slice[si]==no_slice)
  {
    init_element_direct(he,si,slices[si].u.pipe.u.branch.length);
    result = slice_traverse_children(si,st);
  }
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while initialising a hash table element
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean init_element_help_hashed(slice_index si, slice_traversal *st)
{
  boolean result;
  dhtElement * const he = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_element_help(he,si);
  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Traverse a slice while initialising a hash table element
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 * @return result of traversing si's children
 */
boolean init_element_branch_ser(slice_index si, slice_traversal *st)
{
  boolean result;
  dhtElement * const he = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_element_series(he,si);
  result = slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const element_initialisers[] =
{
  &init_element_branch_d,    /* STBranchDirect */
  &slice_traverse_children,  /* STBranchDirectDefender */
  &slice_traverse_children,  /* STBranchHelp */
  &init_element_branch_ser,  /* STBranchSeries */
  &slice_traverse_children,  /* STBranchFork */
  &init_element_leaf_d,      /* STLeafDirect */
  &init_element_leaf_h,      /* STLeafHelp */
  &init_element_leaf_d,      /* STLeafSelf */
  &slice_operation_noop,     /* STLeafForced */
  &slice_traverse_children,  /* STReciprocal */
  &slice_traverse_children,  /* STQuodlibet */
  &slice_traverse_children,  /* STNot */
  &slice_traverse_children,  /* STMoveInverter */
  &slice_traverse_children,  /* STHelpRoot */
  &slice_traverse_children,  /* STHelpAdapter */
  &init_element_help_hashed, /* STHelpHashed */
  &slice_traverse_children   /* STReflexGuard */
};

/* Initialise the bits representing all slices in a hash table
 * element's data field with null values 
 * @param he address of hash table element
 */
static void init_elements(dhtElement *he)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&element_initialisers,he);
  traverse_slices(root_slice,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (attempt to) allocate a hash table element - compress the current
 * hash table if necessary; exit()s if allocation is not possible
 * in spite of compression
 * @param hb has value (basis for calculation of key)
 * @return address of element
 */
static dhtElement *allocDHTelement(dhtValue hb)
{
  dhtElement *result= dhtEnterElement(pyhash, (dhtValue)hb, 0);
  unsigned long nrKeys = dhtKeyCount(pyhash);
  while (result==dhtNilElement)
  {
    compresshash();
    if (dhtKeyCount(pyhash)==nrKeys)
    {
      /* final attempt */
      inithash();
      result = dhtEnterElement(pyhash, (dhtValue)hb, 0);
      break;
    }
    else
    {
      nrKeys = dhtKeyCount(pyhash);
      result = dhtEnterElement(pyhash, (dhtValue)hb, 0);
    }
  }

  if (result==dhtNilElement)
  {
    fprintf(stderr,
            "Sorry, cannot enter more hashelements "
            "despite compression\n");
    exit(-2);
  }

  return result;
}

void addtohash(slice_index si, hashwhat what, hash_value_type val)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",what);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);

  /* TODO create hash slice(s) that are only active if we can
   * allocated the hash table. */
  if (pyhash!=0)
  {
    HashBuffer *hb = &hashBuffers[nbply];
    dhtElement *he = dhtLookupElement(pyhash, (dhtValue)hb);

    if (!isHashBufferValid[nbply])
      (*encode)();

    if (he == dhtNilElement)
    {
      /* the position is new */
      he = allocDHTelement((dhtValue)hb);
      he->Data = template_element.Data;

      switch (what)
      {
        case SerNoSucc:
          set_value_series(he,si,val);
          break;

        case hash_help_insufficient_nr_half_moves:
          set_value_help(he,si,val);
          break;

        case DirSucc:
          set_value_direct_succ(he,si,val);
          break;

        case DirNoSucc:
          set_value_direct_nosucc(he,si,val);
          break;

        default:
          assert(0);
          break;
      }
    }
    else
      switch (what)
      {
        /* TODO use optimized operation? */
        case SerNoSucc:
          if (get_value_series(he,si)<val)
            set_value_series(he,si,val);
          break;

        case hash_help_insufficient_nr_half_moves:
          if (get_value_help(he,si)<val)
            set_value_help(he,si,val);
          break;

        case DirSucc:
          if (get_value_direct_succ(he,si)>val)
            set_value_direct_succ(he,si,val);
          break;

        case DirNoSucc:
          if (get_value_direct_nosucc(he,si)<val)
            set_value_direct_nosucc(he,si,val);
          break;

        default:
          assert(0);
          break;
      }
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
} /* addtohash */

static unsigned long hashtable_kilos;

unsigned long allochash(unsigned long nr_kilos)
{
#if defined(FXF)
  unsigned long const one_kilo = 1<<10;
  while (fxfInit(nr_kilos*one_kilo)==-1)
    /* we didn't get hashmemory ... */
    nr_kilos /= 2;
  ifTESTHASH(fxfInfo(stdout));
#endif /*FXF*/

  hashtable_kilos = nr_kilos;

  return nr_kilos;
}

void inithash(void)
{
  int Small, Large;
  int i, j;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ifTESTHASH(
      sprintf(GlobalStr, "calling inithash\n");
      StdString(GlobalStr)
      );

#if defined(__unix) && defined(TESTHASH)
  OldBreak= sbrk(0);
#endif /*__unix,TESTHASH*/

  is_there_slice_with_nonstandard_min_length = false;

  compression_counter = 0;

  init_slice_properties();
  template_element.Data = 0;
  init_elements(&template_element);

  dhtRegisterValue(dhtBCMemValue, 0, &dhtBCMemoryProcs);
  dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
  pyhash= dhtCreate(dhtBCMemValue, dhtCopy, dhtSimpleValue, dhtNoCopy);
  if (pyhash==0)
  {
    TraceValue("%s\n",dhtErrorMsg());
  }

  ifHASHRATE(use_pos = use_all = 0);

  /* check whether a piece can be coded in a single byte */
  j = 0;
  for (i = PieceCount; Empty < i; i--)
    if (exist[i])
      piece_nbr[i] = j++;

  if (CondFlag[haanerchess])
    piece_nbr[obs]= j++;

  one_byte_hash = j<(1<<(CHAR_BIT/2)) && PieSpExFlags<(1<<(CHAR_BIT/2));

  bytes_per_spec= 1;
  if ((PieSpExFlags >> CHAR_BIT) != 0)
    bytes_per_spec++;
  if ((PieSpExFlags >> 2*CHAR_BIT) != 0)
    bytes_per_spec++;

  bytes_per_piece= one_byte_hash ? 1 : 1+bytes_per_spec;

  if (isIntelligentModeActive)
  {
    one_byte_hash = false;
    bytes_per_spec= 5; /* TODO why so high??? */
  }

  if (slices[1].u.leaf.goal==goal_proof
      || slices[1].u.leaf.goal==goal_atob)
  {
    encode = ProofEncode;
    if (hashtable_kilos>0 && MaxPositions==0)
      MaxPositions= hashtable_kilos/(24+sizeof(char *)+1);
  }
  else
  {
    Small= TellSmallEncodePosLeng();
    Large= TellLargeEncodePosLeng();
    if (Small <= Large) {
      encode= SmallEncode;
      if (hashtable_kilos>0 && MaxPositions==0)
        MaxPositions= hashtable_kilos/(Small+sizeof(char *)+1);
    }
    else
    {
      encode= LargeEncode;
      if (hashtable_kilos>0 && MaxPositions==0)
        MaxPositions= hashtable_kilos/(Large+sizeof(char *)+1);
    }
  }

#if defined(FXF)
  ifTESTHASH(printf("MaxPositions: %7lu\n", MaxPositions));
  assert(hashtable_kilos/1024<UINT_MAX);
  ifTESTHASH(printf("hashtable_kilos:    %7u KB\n",
                    (unsigned int)(hashtable_kilos/1024)));
#else
  ifTESTHASH(
      printf("room for up to %lu positions in hash table\n", MaxPositions));
#endif /*FXF*/

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* inithash */

void closehash(void)
{
#if defined(TESTHASH)
  sprintf(GlobalStr, "calling closehash\n");
  StdString(GlobalStr);

#if defined(HASHRATE)
  sprintf(GlobalStr, "%ld enquiries out of %ld successful. ",
          use_pos, use_all);
  StdString(GlobalStr);
  if (use_all) {
    sprintf(GlobalStr, "Makes %ld%%\n", (100 * use_pos) / use_all);
    StdString(GlobalStr);
  }
#endif
#if defined(__unix)
  {
#if defined(FXF)
    unsigned long const HashMem = fxfTotal();
#else
    unsigned long const HashMem = sbrk(0)-OldBreak;
#endif /*FXF*/
    unsigned long const HashCount = pyhash==0 ? 0 : dhtKeyCount(pyhash);
    if (HashCount>0)
    {
      unsigned long const BytePerPos = (HashMem*100)/HashCount;
      sprintf(GlobalStr,
              "Memory for hash-table: %ld, "
              "gives %ld.%02ld bytes per position\n",
              HashMem, BytePerPos/100, BytePerPos%100);
    }
    else
      sprintf(GlobalStr, "Nothing in hashtable\n");
    StdString(GlobalStr);
#endif /*__unix*/
  }
#endif /*TESTHASH*/

  /* TODO create hash slice(s) that are only active if we can
   * allocated the hash table. */
  if (pyhash!=0)
  {
    dhtDestroy(pyhash);
    pyhash = 0;
  }

#if defined(TESTHASH) && defined(FXF)
  fxfInfo(stdout);
#endif /*TESTHASH,FXF*/
} /* closehash */

/* assert()s below this line must remain active even in "productive"
 * executables. */
#undef NDEBUG
#include <assert.h>

/* Check assumptions made in the hashing module. Abort if one of them
 * isn't met.
 * This is called from checkGlobalAssumptions() once at program start.
 */
void check_hash_assumptions(void)
{
  /* SmallEncode uses 1 byte for both row and file of a square */
  assert(nr_rows_on_board<1<<(CHAR_BIT/2));
  assert(nr_files_on_board<1<<(CHAR_BIT/2));

  /* LargeEncode() uses 1 bit per square */
  assert(nr_files_on_board<=CHAR_BIT);

  /* the encoding functions encode Flags as 2 bytes */
  assert(PieSpCount<=2*CHAR_BIT);
}
