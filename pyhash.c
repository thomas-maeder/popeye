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
#include "pyhelp.h"
#include "pyseries.h"
#include "pynontrv.h"
#include "stipulation/branch.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "trace.h"

typedef unsigned int hash_value_type;

static struct dht *pyhash;

static char    piece_nbr[PieceCount];
static boolean one_byte_hash;
static unsigned int bytes_per_spec;
static unsigned int bytes_per_piece;

static boolean is_there_slice_with_nonstandard_min_length;

/* Minimal value of a hash table element.
 * compresshash() will remove all elements with a value less than
 * minimalElementValueAfterCompression, and increase
 * minimalElementValueAfterCompression if necessary.
 */
static hash_value_type minimalElementValueAfterCompression;


/* Container of indices of hash slices
 */
static unsigned int nr_hash_slices;
static slice_index hash_slices[max_nr_slices];


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

/* hash table element type defining the data member as we use it in
 * this module
 */
typedef struct
{
	dhtValue Key;
    data_type data;
} element_t;

/* Grand union of "element" type and the generic one used by the hash
 * table implementation.
 * Using this union type rather than casting frm dhtElement * to
 * element_t * avoids aliasing issues.
 */
typedef union
{
    dhtElement d;
    element_t e;
} hashElement_union_t;

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
            slice_index anchor;
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
  &slice_traverse_children,       /* STProxy */
  &slice_property_offset_shifter, /* STBranchDirect */
  &slice_property_offset_shifter, /* STBranchDirectDefender */
  &slice_property_offset_shifter, /* STBranchHelp */
  &slice_property_offset_shifter, /* STHelpFork */
  &slice_property_offset_shifter, /* STBranchSeries */
  &slice_property_offset_shifter, /* STSeriesFork */
  &slice_property_offset_shifter, /* STLeafDirect */
  &slice_property_offset_shifter, /* STLeafHelp */
  &slice_property_offset_shifter, /* STLeafForced */
  &slice_property_offset_shifter, /* STReciprocal */
  &slice_property_offset_shifter, /* STQuodlibet */
  &slice_property_offset_shifter, /* STNot */
  &slice_property_offset_shifter, /* STMoveInverter */
  &slice_property_offset_shifter, /* STDirectRoot */
  &slice_property_offset_shifter, /* STDirectDefenderRoot */
  &slice_property_offset_shifter, /* STDirectHashed */
  &slice_property_offset_shifter, /* STHelpRoot */
  &slice_property_offset_shifter, /* STHelpHashed */
  &slice_property_offset_shifter, /* STSeriesRoot */
  &slice_property_offset_shifter, /* STParryFork */
  &slice_property_offset_shifter, /* STSeriesHashed */
  &slice_property_offset_shifter, /* STSelfCheckGuard */
  &slice_property_offset_shifter, /* STDirectDefense */
  &slice_property_offset_shifter, /* STReflexHelpFilter */
  &slice_property_offset_shifter, /* STReflexGuard */
  &slice_property_offset_shifter, /* STReflexAttackerFilter */
  &slice_property_offset_shifter, /* STReflexDefenderFilter */
  &slice_property_offset_shifter, /* STSelfAttack */
  &slice_property_offset_shifter, /* STSelfDefense */
  &slice_property_offset_shifter, /* STRestartGuard */
  &slice_property_offset_shifter, /* STGoalReachableGuard */
  &slice_property_offset_shifter, /* STKeepMatingGuard */
  &slice_property_offset_shifter, /* STMaxFlightsquares */
  &slice_property_offset_shifter, /* STDegenerateTree */
  &slice_property_offset_shifter, /* STMaxNrNonTrivial */
  &slice_property_offset_shifter  /* STMaxThreatLength */
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
 * current stipulation slices
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for si and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_direct_root(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a direct branch
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for branch and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_hashed_direct(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;

  /* TODO This is a bit of a hack - we are hashing for a leaf -> no
   * help adapter has adjusted the valueOffset!
   */
  if (slices[slices[si].u.pipe.next].type==STLeafDirect)
    --sis->valueOffset;

  init_slice_property_direct(si,length,sis);
  hash_slices[nr_hash_slices++] = si;
  slice_traverse_children(si,st);

  return result;
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 * @return true iff the properties for si and its children have been
 *         successfully initialised
 */
static boolean init_slice_properties_hashed_help(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;
  unsigned int const length = slices[si].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[slices[si].u.pipe.next].type==STLeafHelp)
    --sis->valueOffset;
  else
  {
    slice_index const sibling = branch_find_slice(STHelpHashed,si);
    
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    unsigned int const width = bit_width((length-slack_length_help+1)/2);

    sis->valueOffset -= width;

    if (sibling!=no_slice
        && slices[sibling].u.pipe.u.branch.length>slack_length_help
        && get_slice_traversal_slice_state(sibling,st)==slice_not_traversed)
    {
      /* 1 bit more because we have two slices whose values are added
       * for computing the value of this branch */
      --sis->valueOffset;

      traverse_slices(sibling,st);
    }
  }

  init_slice_property_help(si,length-slack_length_help,sis);
  slice_traverse_children(si,st);

  hash_slices[nr_hash_slices++] = si;
    
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
static boolean init_slice_properties_hashed_series(slice_index si,
                                                   slice_traversal *st)
{
  boolean const result = true;
  slice_initializer_state * const sis = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_slice_property_series(si,length-slack_length_series,sis);
  hash_slices[nr_hash_slices++] = si;

  slice_traverse_children(si,st);
  traverse_slices(towards_goal,st);

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
static boolean init_slice_properties_series_root(slice_index si,
                                                 slice_traversal *st)
{
  boolean const result = true;
  slice_index const towards_goal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);
  traverse_slices(towards_goal,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const slice_properties_initalisers[] =
{
  &slice_traverse_children,              /* STProxy */
  &init_slice_properties_pipe,           /* STBranchDirect */
  &init_slice_properties_pipe,           /* STBranchDirectDefender */
  &init_slice_properties_pipe,           /* STBranchHelp */
  &slice_traverse_children,              /* STHelpFork */
  &init_slice_properties_pipe,           /* STBranchSeries */
  &slice_traverse_children,              /* STSeriesFork */
  &slice_operation_noop,                 /* STLeafDirect */
  &slice_operation_noop,                 /* STLeafHelp */
  &slice_operation_noop,                 /* STLeafForced */
  &init_slice_properties_fork,           /* STReciprocal */
  &init_slice_properties_fork,           /* STQuodlibet */
  &init_slice_properties_pipe,           /* STNot */
  &init_slice_properties_pipe,           /* STMoveInverter */
  &init_slice_properties_direct_root,    /* STDirectRoot */
  &init_slice_properties_direct_root,    /* STDirectDefenderRoot */
  &init_slice_properties_hashed_direct,  /* STDirectHashed */
  &slice_traverse_children,              /* STHelpRoot */
  &init_slice_properties_hashed_help,    /* STHelpHashed */
  &init_slice_properties_series_root,    /* STSeriesRoot */
  &slice_traverse_children,              /* STParryFork */
  &init_slice_properties_hashed_series,  /* STSeriesHashed */
  &init_slice_properties_pipe,           /* STSelfCheckGuard */
  &slice_traverse_children,              /* STDirectDefense */
  &slice_traverse_children,              /* STReflexHelpFilter */
  &slice_traverse_children,              /* STReflexGuard */
  &slice_traverse_children,              /* STReflexAttackerFilter */
  &slice_traverse_children,              /* STReflexDefenderFilter */
  &init_slice_properties_direct_root,    /* STSelfAttack */
  &init_slice_properties_direct_root,    /* STSelfDefense */
  &init_slice_properties_pipe,           /* STRestartGuard */
  &init_slice_properties_pipe,           /* STGoalReachableGuard */
  &init_slice_properties_pipe,           /* STKeepMatingGuard */
  &init_slice_properties_pipe,           /* STMaxFlightsquares */
  &init_slice_properties_pipe,           /* STDegenerateTree */
  &init_slice_properties_pipe,           /* STMaxNrNonTrivial */
  &init_slice_properties_pipe            /* STMaxThreatLength */
};

static boolean find_slice_with_nonstandard_min_length(void)
{
  boolean result = false;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_hash_slices && !result; ++i)
  {
    slice_index const si = hash_slices[i];
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    switch (slices[si].type)
    {
      case STDirectHashed:
        result = min_length==length && length>slack_length_direct+1;
        break;

      case STHelpHashed:
        result = min_length==length && length>slack_length_help+1;
        break;

      case STSeriesHashed:
        result = min_length==length && length>slack_length_series+1;
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Reduce the value offsets for the hash slices to the minimal
 * possible value. This is important in order for
 * minimalElementValueAfterCompression not to grow too high in
 * compresshash().
 */
static void minimiseValueOffset(void)
{
  unsigned int minimalValueOffset = sizeof(data_type)*CHAR_BIT;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_hash_slices; ++i)
  {
    slice_index const si = hash_slices[i];
    unsigned int const valueOffset = slice_properties[si].valueOffset;
    if (valueOffset<minimalValueOffset)
      minimalValueOffset = valueOffset;
  }

  TraceValue("%u\n",minimalValueOffset);

  for (i = 0; i<nr_hash_slices; ++i)
  {
    slice_index const si = hash_slices[i];
    slice_properties[si].valueOffset -= minimalValueOffset;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nr_hash_slices = 0;

  slice_traversal_init(&st,&slice_properties_initalisers,&sis);
  traverse_slices(root_slice,&st);

  is_there_slice_with_nonstandard_min_length
      = find_slice_with_nonstandard_min_length();

  minimiseValueOffset();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Pseudo hash table element - template for fast initialization of
 * newly created actual table elements
 */
static hashElement_union_t template_element;


static void set_value_direct_nosucc(hashElement_union_t *hue,
                                    slice_index si,
                                    hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t * const e = &hue->e;
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

static void set_value_direct_succ(hashElement_union_t *hue,
                                  slice_index si,
                                  hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t * const e = &hue->e;
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

static void set_value_help(hashElement_union_t *hue,
                           slice_index si,
                           hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.h.maskNoSucc;
  element_t * const e = &hue->e;
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

static void set_value_series(hashElement_union_t *hue,
                             slice_index si,
                             hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.s.offsetNoSucc;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.s.maskNoSucc;
  element_t * const e = &hue->e;
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

static hash_value_type get_value_direct_succ(hashElement_union_t const *hue,
                                             slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t const * const e = &hue->e;
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

static hash_value_type get_value_direct_nosucc(hashElement_union_t const *hue,
                                               slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t const * const e = &hue->e;
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

static hash_value_type get_value_help(hashElement_union_t const *hue,
                                      slice_index si)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSucc;
  unsigned int const  mask = slice_properties[si].u.h.maskNoSucc;
  element_t const * const e = &hue->e;
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

static hash_value_type get_value_series(hashElement_union_t const *hue,
                                        slice_index si)
{
  unsigned int const offset = slice_properties[si].u.s.offsetNoSucc;
  unsigned int const mask = slice_properties[si].u.s.maskNoSucc;
  element_t const * const e = &hue->e;
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
 * @return value of contribution of slice si to *he's value
 */
static hash_value_type own_value_of_data_direct(hashElement_union_t const *hue,
                                                slice_index si)
{
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  hash_value_type result;
  hash_value_type succ;
  hash_value_type nosucc;
  hash_value_type succ_neg;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  succ = get_value_direct_succ(hue,si);
  nosucc = get_value_direct_nosucc(hue,si);

  assert(succ<=length);
  succ_neg = length-succ;

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
static hash_value_type own_value_of_data_help(hashElement_union_t const *hue,
                                              slice_index si)
{
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = get_value_help(hue,si);

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
static hash_value_type own_value_of_data_series(hashElement_union_t const *hue,
                                                slice_index si)
{
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = get_value_series(hue,si);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of a slice to the value of a hash table
 * element node.
 * @param he address of hash table element to determine value of
 * @param si slice index
 * @return value of contribuation of the slice to *he's value
 */
static hash_value_type value_of_data_from_slice(hashElement_union_t const *hue,
                                                slice_index si)
{
  hash_value_type result;
  unsigned int const offset = slice_properties[si].valueOffset;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p ",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type," ");
  TraceValue("%u\n",slice_properties[si].valueOffset);

  switch (slices[si].type)
  {
    case STDirectHashed:
      result = own_value_of_data_direct(hue,si) << offset;
      break;

    case STHelpHashed:
      result = own_value_of_data_help(hue,si) << offset;
      break;

    case STSeriesHashed:
      result = own_value_of_data_series(hue,si) << offset;
      break;

    default:
      assert(0);
      result = 0;
      break;
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
static hash_value_type value_of_data(hashElement_union_t const *hue)
{
  hash_value_type result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_hash_slices; ++i)
    result += value_of_data_from_slice(hue,hash_slices[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x",result);
  TraceFunctionResultEnd();
  return result;
}

#if defined(TESTHASH)
static unsigned long totalRemoveCount = 0;
#endif

static void compresshash (void)
{
  dhtElement const *he;
  unsigned long targetKeyCount;
#if defined(TESTHASH)
  unsigned long RemoveCnt = 0;
  unsigned long initCnt;
  unsigned long visitCnt;
  unsigned long runCnt;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  targetKeyCount = dhtKeyCount(pyhash);
  targetKeyCount -= targetKeyCount/16;

  TraceValue("%u",minimalElementValueAfterCompression);
  TraceValue("%u",dhtKeyCount(pyhash));
  TraceValue("%u\n",targetKeyCount);

#if defined(TESTHASH)
  printf("\nminimalElementValueAfterCompression: %08x\n",
         minimalElementValueAfterCompression);
  fflush(stdout);
  initCnt= dhtKeyCount(pyhash);
  runCnt= 0;
#endif  /* TESTHASH */

  while (true)
  {
#if defined(TESTHASH)
    printf("minimalElementValueAfterCompression: %08x\n",
           minimalElementValueAfterCompression);
    printf("RemoveCnt: %ld\n", RemoveCnt);
    fflush(stdout);
    visitCnt= 0;
#endif  /* TESTHASH */

#if defined(TESTHASH)
    for (he = dhtGetFirstElement(pyhash);
         he!=0;
         he = dhtGetNextElement(pyhash))
      printf("%08x\n",value_of_data(he));
    exit (0);
#endif  /* TESTHASH */

    for (he = dhtGetFirstElement(pyhash);
         he!=0;
         he = dhtGetNextElement(pyhash))
    {
      hashElement_union_t const * const hue = (hashElement_union_t const *)he;
      if (value_of_data(hue)<minimalElementValueAfterCompression)
      {
#if defined(TESTHASH)
        RemoveCnt++;
        totalRemoveCount++;
#endif  /* TESTHASH */

        dhtRemoveElement(pyhash, hue->d.Key);

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

    if (dhtKeyCount(pyhash)<=targetKeyCount)
      break;
    else
      ++minimalElementValueAfterCompression;
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

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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

static unsigned int estimateNumberOfHoles(void)
{
  unsigned int result = 0;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i<nr_hash_slices && !result; ++i)
  {
    slice_index const si = hash_slices[i];
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    switch (slices[si].type)
    {
      case STDirectHashed:
        result += (length-slack_length_direct+1)/2;
        break;

      case STHelpHashed:
        result += (length-slack_length_help+1)/2;
        break;

      case STSeriesHashed:
        result += length-slack_length_series;
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static unsigned int TellCommonEncodePosLeng(unsigned int len,
                                            unsigned int nbr_p)
{
  len++; /* Castling_Flag */

  if (CondFlag[haanerchess])
  {
    unsigned int nbr_holes = estimateNumberOfHoles();
    if (nbr_holes>(nr_files_on_board*nr_rows_on_board-nbr_p)/2)
      nbr_holes = (nr_files_on_board*nr_rows_on_board-nbr_p)/2;
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

static unsigned int TellLargeEncodePosLeng(void)
{
  square const *bnp;
  unsigned int nbr_p = 0;
  unsigned int len = 8;

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

static unsigned int TellSmallEncodePosLeng(void)
{
  square const *bnp;
  unsigned int nbr_p = 0;
  unsigned int len = 0;

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
extern size_t fxfMINSIZE;

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

/* Initialise the bits representing all slices in a hash table
 * element's data field with null values 
 * @param he address of hash table element
 */
static void init_elements(hashElement_union_t *hue)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_hash_slices; ++i)
  {
    slice_index const si = hash_slices[i];
    switch (slices[si].type)
    {
      case STDirectHashed:
        set_value_direct_nosucc(hue,si,0);
        set_value_direct_succ(hue,si,slices[si].u.pipe.u.branch.length/2);
        break;

      case STHelpHashed:
        set_value_help(hue,si,0);
        break;

      case STSeriesHashed:
        set_value_series(hue,si,0);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* (attempt to) allocate a hash table element - compress the current
 * hash table if necessary; exit()s if allocation is not possible
 * in spite of compression
 * @param hb has value (basis for calculation of key)
 * @return address of element
 */
static dhtElement *allocDHTelement(dhtConstValue hb)
{
  dhtElement *result = dhtEnterElement(pyhash,hb,template_element.d.Data);
  while (result==dhtNilElement)
  {
    unsigned long const nrKeysBeforeCompression = dhtKeyCount(pyhash);
    compresshash();
    if (dhtKeyCount(pyhash)==nrKeysBeforeCompression)
    {
      dhtDestroy(pyhash);
      fxfReset();
      pyhash = dhtCreate(dhtBCMemValue,dhtCopy,dhtSimpleValue,dhtNoCopy);
      assert(pyhash!=0);
      result = dhtEnterElement(pyhash,hb,template_element.d.Data);
      break;
    }
    else
      result = dhtEnterElement(pyhash,hb,template_element.d.Data);
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

static unsigned long hashtable_kilos;

/* Allocate memory for the hash table. If the requested amount of
 * memory isn't available, reduce the amount until allocation
 * succeeds. 
 * @param nr_kilos number of kilo-bytes to allocate
 * @return number of kilo-bytes actually allocated
 */
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

  pyhash = dhtCreate(dhtBCMemValue,dhtCopy,dhtSimpleValue,dhtNoCopy);
  if (pyhash==0)
  {
    TraceValue("%s\n",dhtErrorMsg());
    return 0;
  }
  else
    return nr_kilos;
}

/* Determine whether the hash table has been successfully allocated
 * @return true iff the hashtable has been allocated
 */
boolean is_hashtable_allocated(void)
{
  return pyhash!=0;
}

void inithash(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ifTESTHASH(
      sprintf(GlobalStr, "calling inithash\n");
      StdString(GlobalStr)
      );

  if (pyhash!=0)
  {
    int i, j;

#if defined(__unix) && defined(TESTHASH)
    OldBreak= sbrk(0);
#endif /*__unix,TESTHASH*/

    minimalElementValueAfterCompression = 2;

    init_slice_properties();

    template_element.d.Data = 0;
    init_elements(&template_element);

    dhtRegisterValue(dhtBCMemValue,0,&dhtBCMemoryProcs);
    dhtRegisterValue(dhtSimpleValue,0,&dhtSimpleProcs);

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
      unsigned int const Small = TellSmallEncodePosLeng();
      unsigned int const Large = TellLargeEncodePosLeng();
      if (Small<=Large)
      {
        encode = SmallEncode;
        if (hashtable_kilos>0 && MaxPositions==0)
          MaxPositions= hashtable_kilos/(Small+sizeof(char *)+1);
      }
      else
      {
        encode = LargeEncode;
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

    dhtDestroy(pyhash);

#if defined(TESTHASH) && defined(FXF)
    fxfInfo(stdout);
#endif /*TESTHASH,FXF*/

    fxfReset();

    pyhash = dhtCreate(dhtBCMemValue,dhtCopy,dhtSimpleValue,dhtNoCopy);
    assert(pyhash!=0);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* inithash */

void closehash(void)
{
  if (pyhash!=0)
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
  }
} /* closehash */

/* Allocate a STDirectHashed slice
 * @param length maximal number of half moves until goal
 * @param min_length minimal number of half moves until goal
 * @param proxy_to_goal identifies proxy slice leading towards gaol
 * @return identifier of allocated slice
 */
static slice_index alloc_direct_hashed_slice(stip_length_type length,
                                             stip_length_type min_length,
                                             slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch(STDirectHashed,length,min_length,proxy_to_goal);
                        
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STDirectHashed slice for a ST{Branch,Leaf}Direct slice
 * and insert it before the slice
 * @param si identifies ST{Branch,Leaf}Direct slice
 */
void insert_directhashed_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  switch (slices[si].type)
  {
    case STLeafDirect:
    {
      slice_index const hash = alloc_direct_hashed_slice(slack_length_direct+1,
                                                         slack_length_direct+1,
                                                         no_slice);
      slice_index const prev = slices[si].prev;
      branch_link(hash,si);
      branch_link(prev,hash);
      break;
    }

    case STBranchDirect:
    {
      stip_length_type const length = slices[si].u.pipe.u.branch.length;
      stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
      slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
      slice_index const hash = alloc_direct_hashed_slice(length,
                                                         min_length,
                                                         to_goal);
      slice_index const prev = slices[si].prev;
      branch_link(hash,si);
      branch_link(prev,hash);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STHelpHashed slice
 * @param length maximal number of half moves until goal
 * @param min_length minimal number of half moves until goal
 * @param proxy_to_goal identifies proxy slice leading towards gaol
 * @return identifier of allocated slice
 */
static slice_index alloc_help_hashed_slice(stip_length_type length,
                                           stip_length_type min_length,
                                           slice_index proxy_to_goal)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",proxy_to_goal);
  TraceFunctionParamListEnd();

  result = alloc_branch(STHelpHashed,length,min_length,proxy_to_goal);
                        
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STHelpHashed slice for a ST{Branch,Leaf}Help slice
 * and insert it before the slice
 * @param si identifies ST{Branch,Leaf}Help slice
 */
void insert_helphashed_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");

  switch (slices[si].type)
  {
    case STLeafHelp:
    {
      slice_index const hash = alloc_help_hashed_slice(slack_length_help+1,
                                                       slack_length_help+1,
                                                       no_slice);
      slice_index const prev = slices[si].prev;
      branch_link(hash,si);
      branch_link(prev,hash);
      break;
    }

    case STBranchHelp:
    {
      stip_length_type const length = slices[si].u.pipe.u.branch.length;
      stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
      slice_index const to_goal = slices[si].u.pipe.u.branch.towards_goal;
      slice_index const hash = alloc_help_hashed_slice(length,
                                                       min_length,
                                                       to_goal);
      slice_index const prev = slices[si].prev;
      branch_link(hash,si);
      branch_link(prev,hash);
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STSeriesHashed slice for a STBranchSeries slice
 * and insert it before the slice
 * @param si identifies STBranchSeries slice
 */
void insert_serieshashed_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(SliceType,slices[si].type,"\n");
  assert(slices[si].type==STBranchSeries);

  {
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
    slice_index const proxy_to_goal = slices[si].u.pipe.u.branch.towards_goal;
    slice_index const hash = alloc_branch(STSeriesHashed,
                                          length,min_length,
                                          proxy_to_goal);
    slice_index const prev = slices[si].prev;
    branch_link(hash,si);
    branch_link(prev,hash);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that the current position does not have a solution in a
 * number of half-moves
 * @param si index of slice where the current position was reached
 * @param n number of half-moves
 */
static void addtohash_dir_nosucc(slice_index si, stip_length_type n)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = n/2;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  assert(isHashBufferValid[nbply]);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_direct_nosucc(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_direct_nosucc(hue,si)<val)
      set_value_direct_nosucc(hue,si,val);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
}

/* Remember that the current position has a solution in a number of
 * half-moves
 * @param si index of slice where the current position was reached
 * @param n number of half-moves
 */
static void addtohash_dir_succ(slice_index si, stip_length_type n)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = n/2-1;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  assert(isHashBufferValid[nbply]);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_direct_succ(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_direct_succ(hue,si)>val)
      set_value_direct_succ(hue,si,val);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
}

/* Adjust to the knowledge in the hash table the minimal number half
 * moves to be tried in some solving operation.
 * @param si slice index
 * @param n maximum number of half moves allowed for the operation
 * @param n_min minimal number of half moves to be tried
 * @return adjusted n_min; n+2 if no solving is useful at all
 */
static stip_length_type adjust_n_min(slice_index si,
                                     stip_length_type n,
                                     stip_length_type n_min)
{
  stip_length_type result = n_min;
  HashBuffer * const hb = &hashBuffers[nbply];
  dhtElement const *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  if (!isHashBufferValid[nbply])
    (*encode)();

  he = dhtLookupElement(pyhash,hb);
  if (he!=dhtNilElement)
  {
    hashElement_union_t const * const hue = (hashElement_union_t const *)he;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

    hash_value_type const val_nosucc = n/2;
    hash_value_type const nosucc = get_value_direct_nosucc(hue,si);
    if (nosucc>=val_nosucc && nosucc<=val_nosucc+length-min_length)
      result = n+2;
    else
    {
      stip_length_type const n_min_new = 2*nosucc+n_min%2;
      if (result<n_min_new)
        result = n_min_new;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* Solve a slice
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return number of half moves effectively used
 *         n+2 if no solution was found
 *         (n-slack_length_direct)%2 if the previous move led to a
 *            dead end (e.g. self-check)
 */
stip_length_type direct_hashed_solve_in_n(slice_index si,
                                          stip_length_type n,
                                          stip_length_type n_min)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  n_min = adjust_n_min(si,n,n_min);
  if (n_min<=n)
  {
    result = direct_solve_in_n(slices[si].u.pipe.next,n,n_min);
    if (result<=n)
      addtohash_dir_succ(si,result);
    else
      addtohash_dir_nosucc(si,n);
  }
  else
    result = n+2;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write continuations after the defense just played.
 * We know that there is at least 1 continuation to the defense.
 * Only continuations of minimal length are looked for and written.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_min minimal number of half moves to try
 */
void direct_hashed_solve_continuations_in_n(slice_index si,
                                            stip_length_type n,
                                            stip_length_type n_min)
{
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  n_min = adjust_n_min(si,n,n_min);
  assert(n_min<=n);

  direct_solve_continuations_in_n(next,n,n_min);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine and write the threats after the move that has just been
 * played.
 * @param threats table where to add threats
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @param n_min minimal number of half moves to try
 * @return length of threats
 *         (n-slack_length_direct)%2 if the attacker has something
 *           stronger than threats (i.e. has delivered check)
 *         n+2 if there is no threat
 */
stip_length_type direct_hashed_solve_threats_in_n(table threats,
                                                  slice_index si,
                                                  stip_length_type n,
                                                  stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  result = adjust_n_min(si,n,n_min);
  if (result<=n)
  {
    result = direct_solve_threats_in_n(threats,next,n,n_min);
    if (table_length(threats)>0)
      addtohash_dir_succ(si,result);
    else
      addtohash_dir_nosucc(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
                                               stip_length_type n)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;
  stip_length_type len_threat_min;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  len_threat_min = adjust_n_min(si,len_threat,len_threat);
  if (len_threat_min<=len_threat)
    result = direct_are_threats_refuted_in_n(threats,len_threat,next,n);
  else
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static stip_length_type delegate_has_solution_in_n(slice_index si,
                                                   stip_length_type n,
                                                   stip_length_type n_min)
{
  stip_length_type result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  result = direct_has_solution_in_n(next,n,n_min);
  if (result<=n)
    addtohash_dir_succ(si,result);
  else
    addtohash_dir_nosucc(si,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

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
                                                 stip_length_type n_min)
{
  stip_length_type result;
  HashBuffer * const hb = &hashBuffers[nbply];
  dhtElement const *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",n_min);
  TraceFunctionParamListEnd();

  assert(n%2==slices[si].u.pipe.u.branch.length%2);

  if (!isHashBufferValid[nbply])
    (*encode)();

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
    result = delegate_has_solution_in_n(si,n,n_min);
  else
  {
    hashElement_union_t const * const hue = (hashElement_union_t const *)he;
    stip_length_type const length = slices[si].u.pipe.u.branch.length;
    stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

    /* It is more likely that a position has no solution. */
    /* Therefore let's check for "no solution" first.  TLi */
    hash_value_type const val_nosucc = n/2;
    hash_value_type const nosucc = get_value_direct_nosucc(hue,si);
    if (nosucc>=val_nosucc && nosucc<=val_nosucc+length-min_length)
      result = n+2;
    else
    {
      hash_value_type const val_succ = n/2-1;
      hash_value_type const succ = get_value_direct_succ(hue,si);
      if (succ<=val_succ && succ+length-min_length>=val_succ)
        result = (succ+1)*2 + n%2;
      else
      {
        stip_length_type const n_min_new = 2*nosucc+n_min%2;
        if (n_min<n_min_new)
          n_min = n_min_new;
        result = delegate_has_solution_in_n(si,n,n_min);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Look up whether the current position in the hash table to find out
 * if it has a solution in a number of half-moves
 * @param si index slice where current position was reached
 * @param n number of half-moves
 * @return true iff we know that the current position has no solution
 *         in n half-moves
 */
static boolean inhash_help(slice_index si, stip_length_type n)
{
  boolean result;
  HashBuffer *hb = &hashBuffers[nbply];
  dhtElement const *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);

  if (!isHashBufferValid[nbply])
    (*encode)();

  ifHASHRATE(use_all++);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
    result = false;
  else
  {
    hashElement_union_t const * const hue = (hashElement_union_t const *)he;
    hash_value_type const val = (n+1-slack_length_help)/2;
    hash_value_type const nosucc = get_value_help(hue,si);
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
  
  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remember that the current position does not have a solution in a
 * number of half-moves
 * @param si index of slice where the current position was reached
 * @param n number of half-moves
 */
static void addtohash_help(slice_index si, stip_length_type n)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = (n+1-slack_length_help)/2;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  assert(isHashBufferValid[nbply]);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_help(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_help(hue,si)<val)
      set_value_help(hue,si,val);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean hashed_help_root_solve(slice_index si)
{
  boolean result;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (inhash_help(si,n))
    result = false;
  else if (help_root_solve(slices[si].u.pipe.next))
    result = true;
  else
  {
    result = false;
    addtohash_help(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_help_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (inhash_help(si,n))
    result = false;
  else if (help_solve_in_n(slices[si].u.pipe.next,n))
    result = true;
  else
  {
    result = false;
    addtohash_help(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_help_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (inhash_help(si,n))
    result = false;
  else
  {
    if (help_has_solution_in_n(slices[si].u.pipe.next,n))
      result = true;
    else
    {
      addtohash_help(si,n);
      result = false;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_help_solve_threats_in_n(table threats,
                                    slice_index si,
                                    stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_help);

  if (!inhash_help(si,n))
  {
    slice_index const next = slices[si].u.pipe.next;
    help_solve_threats_in_n(threats,next,n);
    if (table_length(threats)==0)
      addtohash_help(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Look up whether the current position in the hash table to find out
 * if it has a solution in a number of half-moves
 * @param si index slice where current position was reached
 * @param n number of half-moves
 * @return true iff we know that the current position has no solution
 *         in n half-moves
 */
static boolean inhash_series(slice_index si, stip_length_type n)
{
  boolean result;
  HashBuffer *hb = &hashBuffers[nbply];
  dhtElement const *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);

  if (!isHashBufferValid[nbply])
    (*encode)();

  ifHASHRATE(use_all++);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
    result = false;
  else
  {
    hashElement_union_t const * const hue = (hashElement_union_t const *)he;
    hash_value_type const val = n-slack_length_series;
    hash_value_type const nosucc = get_value_series(hue,si);
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

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remember that the current position does not have a solution in a
 * number of half-moves
 * @param si index of slice where the current position was reached
 * @param n number of half-moves
 */
static void addtohash_series(slice_index si, stip_length_type n)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = n-slack_length_series;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",nbply);
  assert(isHashBufferValid[nbply]);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_series(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_series(hue,si)<val)
      set_value_series(hue,si,val);
  }
  
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
}

/* Solve in a number of half-moves
 * @param si identifies slice
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >=1 solution was found
 */
boolean hashed_series_solve_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  if (inhash_series(si,n))
    result = false;
  else if (series_solve_in_n(slices[si].u.pipe.next,n))
    result = true;
  else
  {
    result = false;
    addtohash_series(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 * @return true iff >= 1 solution has been found
 */
boolean hashed_series_has_solution_in_n(slice_index si, stip_length_type n)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  if (inhash_series(si,n))
    result = false;
  else
  {
    if (series_has_solution_in_n(slices[si].u.pipe.next,n))
      result = true;
    else
    {
      addtohash_series(si,n);
      result = false;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write threats
 * @param threats table where to add first moves
 * @param si slice index of slice being solved
 * @param n exact number of half moves until end state has to be reached
 */
void hashed_series_solve_threats_in_n(table threats,
                                      slice_index si,
                                      stip_length_type n)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>slack_length_series);

  if (!inhash_series(si,n))
  {
    slice_index const next = slices[si].u.pipe.next;
    series_solve_threats_in_n(threats,next,n);
    if (table_length(threats)==0)
      addtohash_series(si,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

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
