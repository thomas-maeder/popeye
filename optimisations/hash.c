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
 ** about en_passant_multistep_over[0]-captures, Duellants and Imitators are coded.
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
 ** When there is no more memory, or more than hash_max_number_storable_positions positions
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

#include "debugging/assert.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <memory.h>
#include "optimisations/hash.h"
#include "output/plaintext/message.h"
#include "solving/proofgames.h"
#include "DHT/dhtvalue.h"
#include "DHT/dht.h"
#include "pieces/walks/pawns/en_passant.h"
#include "conditions/bgl.h"
#include "conditions/circe/circe.h"
#include "conditions/circe/parachute.h"
#include "conditions/duellists.h"
#include "conditions/haunted_chess.h"
#include "conditions/imitator.h"
#include "options/nontrivial.h"
#include "solving/avoid_unsolvable.h"
#include "solving/castling.h"
#include "solving/machinery/slack_length.h"
#include "solving/has_solution_type.h"
#include "stipulation/proxy.h"
#include "stipulation/battle_play/branch.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "platform/maxtime.h"
#include "platform/maxmem.h"
#include "solving/pipe.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"
#include "pieces/attributes/neutral/neutral.h"
#include "options/options.h"
#include "conditions/conditions.h"
#include "position/effects/piece_movement.h"

#if defined(HASHRATE)
#include "platform/timer.h"
#endif

unsigned long hash_max_number_storable_positions = ULONG_MAX;

typedef unsigned int hash_value_type;

static struct dht *pyhash;

static char    piece_nbr[nr_piece_walks];
static boolean one_byte_hash;
static unsigned int bytes_per_spec;
static unsigned int bytes_per_piece;

/* TODO we should remove help hash slices instead of testing this flag over and
 * over */
static boolean is_table_uncompressed;

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

#if defined(TESTHASH)
static void dump_hash_buffer(void)
{
  unsigned int const length = hashBuffers[nbply].cmv.Leng;
  unsigned int i;
  for (i = 0; i!=length; ++i)
    printf("%u ",(unsigned int)hashBuffers[nbply].cmv.Data[i]);
  putchar('\n');
}
#endif

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

static byte const black_bit = CHAR_BIT/2 - 1;

static void (*encode)(stip_length_type min_length,
                      stip_length_type validity_value);

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
            unsigned int parity;
            unsigned int offsetNoSucc;
            unsigned int maskNoSucc;
        } h;
    } u;
} slice_properties_t;

static slice_properties_t slice_properties[max_nr_slices];

/* Determine the number of bits necessary to represent a range of numbers
 * @param value maximum value of the range
 * @return number of bits necessary to represent the numbers
 *         from 0 to value (inclusive)
 */
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

typedef struct
{
    unsigned int nrBitsLeft;
    unsigned int valueOffset;
} slice_initializer_state;

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a pipe for which we don't
 * have a more specialised function
 * @param leaf root slice of subtree
 * @param st address of structure defining traversal
 */
static void init_slice_properties_pipe(slice_index pipe,
                                       stip_structure_traversal *st)
{
  slice_index const next = SLICE_NEXT1(pipe);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",pipe);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(pipe,st);
  slice_properties[pipe].valueOffset = slice_properties[next].valueOffset;
  TraceValue("%u",pipe);
  TraceValue("%u",slice_properties[pipe].valueOffset);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shift the value offset of one slice, then continue the traversal
 * @param si identifies the slice currently traversed
 * @param st points to the structure holding the traversal state
 */
static void slice_property_offset_shifter(slice_index si,
                                          stip_structure_traversal *st)
{
  unsigned int const * const delta = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_properties[si].valueOffset -= *delta;

  TraceValue("%u",*delta);
  TraceValue("->%u",slice_properties[si].valueOffset);
  TraceEOL();

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Shift the value offsets of slices reachable from a particular slice
 * @param si identifies slice
 * @param delta indicates how much to shift the value offsets
 */
static void shift_offsets(slice_index si,
                          stip_structure_traversal *st,
                          unsigned int delta)
{
  unsigned int i;
  stip_structure_traversal st_nested;

  stip_structure_traversal_init_nested(&st_nested,st,&delta);
  for (i = 0; i!=nr_slice_structure_types; ++i)
    stip_structure_traversal_override_by_structure(&st_nested,
                                                   i,
                                                   &slice_property_offset_shifter);
  stip_traverse_structure(si,&st_nested);
}

/* Initialise the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a fork
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
static void init_slice_properties_binary(slice_index fork,
                                         stip_structure_traversal *st)
{
  slice_initializer_state * const sis = st->param;

  unsigned int const save_valueOffset = sis->valueOffset;

  slice_index const op1 = SLICE_NEXT1(fork);
  slice_index const op2 = SLICE_NEXT2(fork);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",fork);
  TraceFunctionParamListEnd();

  slice_properties[fork].valueOffset = sis->valueOffset;

  stip_traverse_structure(op1,st);
  sis->valueOffset = save_valueOffset;
  stip_traverse_structure(op2,st);

  TraceValue("%u",op1);
  TraceValue("%u",slice_properties[op1].valueOffset);
  TraceValue("%u",op2);
  TraceValue("%u",slice_properties[op2].valueOffset);
  TraceEOL();

  /* both operand slices must have the same valueOffset, or the
   * shorter one will dominate the longer one */
  if (slice_properties[op1].valueOffset>slice_properties[op2].valueOffset)
  {
    unsigned int const delta = (slice_properties[op1].valueOffset
                                -slice_properties[op2].valueOffset);
    shift_offsets(op1,st,delta);
  }
  else if (slice_properties[op2].valueOffset>slice_properties[op1].valueOffset)
  {
    unsigned int const delta = (slice_properties[op2].valueOffset
                                -slice_properties[op1].valueOffset);
    shift_offsets(op2,st,delta);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the slice_properties array
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
static void init_slice_properties_attack_hashed(slice_index si,
                                                stip_structure_traversal *st)
{
  slice_initializer_state * const sis = st->param;
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;
  unsigned int const size = bit_width((length-min_length+1)/2);
  data_type const mask = (1u<<size)-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",length);
  TraceValue("%u",sis->nrBitsLeft);
  TraceValue("%u",sis->valueOffset);
  TraceEOL();

  sis->valueOffset -= size;
  TraceValue("%u",size);
  TraceValue("->%u",sis->valueOffset);
  TraceEOL();

  slice_properties[si].size = size;
  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u",slice_properties[si].valueOffset);
  TraceEOL();

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.d.offsetNoSucc = sis->nrBitsLeft;
  slice_properties[si].u.d.maskNoSucc = ((sis->nrBitsLeft < (CHAR_BIT * (sizeof mask))) ? (mask << sis->nrBitsLeft) : 0);

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.d.offsetSucc = sis->nrBitsLeft;
  slice_properties[si].u.d.maskSucc = ((sis->nrBitsLeft < (CHAR_BIT * (sizeof mask))) ? (mask << sis->nrBitsLeft) : 0);

  hash_slices[nr_hash_slices++] = si;
  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a slice_properties element representing help play
 * @param si root slice of subtree
 * @param length number of half moves of help slice
 * @param sis state of slice properties initialisation
 */
static void init_slice_property_help(slice_index si,
                                     slice_initializer_state *sis)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;
  unsigned int const size = bit_width((length-min_length+1)/2+1);
  data_type const mask = (1u<<size)-1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_properties[si].size = size;
  slice_properties[si].valueOffset = sis->valueOffset;
  TraceValue("%u",size);
  TraceValue("%08x",mask);
  TraceValue("%u",slice_properties[si].valueOffset);
  TraceEOL();

  assert(sis->nrBitsLeft>=size);
  sis->nrBitsLeft -= size;
  slice_properties[si].u.h.parity = 1-(length-slack_length)%2;
  slice_properties[si].u.h.offsetNoSucc = sis->nrBitsLeft;
  slice_properties[si].u.h.maskNoSucc = mask << sis->nrBitsLeft;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise the slice_properties array
 * @param si root slice of subtree
 * @param st address of structure defining traversal
 */
static void init_slice_properties_hashed_help(slice_index si,
                                              stip_structure_traversal *st)
{
  slice_initializer_state * const sis = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const sibling = branch_find_slice(STHelpHashed,
                                                  si,
                                                  stip_traversal_context_help);

    stip_length_type const length = SLICE_U(si).branch.length;
    unsigned int const width = bit_width((length-slack_length+1)/2);

    if (sibling!=no_slice
        && SLICE_U(sibling).branch.length>slack_length
        && get_stip_structure_traversal_state(sibling,st)==slice_not_traversed)
    {
      assert(sibling!=si);

      /* 1 bit more because we have two slices whose values are added
       * for computing the value of this branch */
      sis->valueOffset -= width+1;

      stip_traverse_structure(sibling,st);
    }

    init_slice_property_help(si,sis);
  }

  stip_traverse_structure_children_pipe(si,st);

  hash_slices[nr_hash_slices++] = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor slice_properties_initalisers[] =
{
  { STAnd,          &init_slice_properties_binary        },
  { STOr,           &init_slice_properties_binary        },
  { STAttackHashed, &init_slice_properties_attack_hashed },
  { STHelpHashed,   &init_slice_properties_hashed_help   }
};

enum
{
  nr_slice_properties_initalisers = (sizeof slice_properties_initalisers
                                     / sizeof slice_properties_initalisers[0])
};

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

  TraceValue("%u",minimalValueOffset);
  TraceEOL();

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
static void init_slice_properties(slice_index si)
{
  stip_structure_traversal st;
  slice_initializer_state sis = {
    sizeof(data_type)*CHAR_BIT,
    sizeof(data_type)*CHAR_BIT
  };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nr_hash_slices = 0;

  stip_structure_traversal_init(&st,&sis);
  stip_structure_traversal_override_by_structure(&st,
                                            slice_structure_pipe,
                                            &init_slice_properties_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &init_slice_properties_pipe);
  stip_structure_traversal_override(&st,
                                    slice_properties_initalisers,
                                    nr_slice_properties_initalisers);
  stip_traverse_structure(si,&st);

  minimiseValueOffset();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* Pseudo hash table element - template for fast initialization of
 * newly created actual table elements
 */
static hashElement_union_t template_element;


static void set_value_attack_nosuccess(hashElement_union_t *hue,
                                       slice_index si,
                                       hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const bits = ((offset < (CHAR_BIT * (sizeof val))) ? (val << offset) : 0);
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t * const e = &hue->e;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();
  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TraceValue("%p",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x",bits);
  TraceEOL();
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x",e->data);
  TraceEOL();
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void set_value_attack_success(hashElement_union_t *hue,
                                     slice_index si,
                                     hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const bits = ((offset < (CHAR_BIT * (sizeof val))) ? (val << offset) : 0);
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t * const e = &hue->e;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",val);
  TraceFunctionParamListEnd();

  TraceValue("%u",slice_properties[si].size);
  TraceValue("%u",offset);
  TraceValue("%08x ",mask);
  TraceValue("%p",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x",bits);
  TraceEOL();
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x",e->data);
  TraceEOL();

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
  TraceValue("0x%08x ",mask);
  TraceValue("0x%08x ",&e->data);
  TraceValue("pre:0x%08x ",e->data);
  TraceValue("0x%08x",bits);
  TraceEOL();
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:0x%08x",e->data);
  TraceEOL();
  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static hash_value_type get_value_attack_success(hashElement_union_t const *hue,
                                                slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t const * const e = &hue->e;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TraceValue("%p",&e->data);
  TraceValue("%08x",e->data);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static hash_value_type get_value_attack_nosuccess(hashElement_union_t const *hue,
                                                  slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetNoSucc;
  unsigned int const mask = slice_properties[si].u.d.maskNoSucc;
  element_t const * const e = &hue->e;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceValue("%08x ",mask);
  TraceValue("%p",&e->data);
  TraceValue("%08x",e->data);
  TraceEOL();

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
  TraceValue("%u",offset);
  TraceValue("0x%08x ",mask);
  TraceValue("0x%08x ",&e->data);
  TraceValue("0x%08x",e->data);
  TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine the contribution of an attacking move to the value of a
 * hash table element node.
 * @param he address of hash table element to determine value of
 * @param si slice index of slice
 * @return value of contribution of slice si to *he's value
 */
static hash_value_type own_value_of_data_solve(hashElement_union_t const *hue,
                                                slice_index si)
{
  stip_length_type const length = SLICE_U(si).branch.length;
  hash_value_type result;
  hash_value_type success;
  hash_value_type nosuccess;
  hash_value_type success_neg;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  success = get_value_attack_success(hue,si);
  nosuccess = get_value_attack_nosuccess(hue,si);

  assert(success<=length);
  success_neg = length-success;

  result = success_neg>nosuccess ? success_neg : nosuccess;

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
  unsigned int const parity = slice_properties[si].u.h.parity;
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* double if parity so that the move not leading to the goal (i.e. played
   * earlier) has more value */
  result = get_value_help(hue,si) << parity;

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
  TraceFunctionParam("%p",hue);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(slice_type,SLICE_TYPE(si));
  TraceValue("%u",slice_properties[si].valueOffset);
  TraceEOL();

  switch (SLICE_TYPE(si))
  {
    case STAttackHashed:
      result = own_value_of_data_solve(hue,si) << offset;
      break;

    case STHelpHashed:
      result = own_value_of_data_help(hue,si) << offset;
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
static unsigned long nrElementsRemovedInAllCompressions = 0;
#endif

static void compresshash (void)
{
  unsigned long targetKeyCount;

#if defined(TESTHASH)
  unsigned long nrElementsRemovedInCompression = 0;
  unsigned long const nrElementsAtStartOfCompression = dhtKeyCount(pyhash);
  unsigned long nrIterationsInCompression = 0;
#endif

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  is_table_uncompressed = false;

  targetKeyCount = dhtKeyCount(pyhash);
  targetKeyCount -= targetKeyCount/16;

  TraceValue("%u",minimalElementValueAfterCompression);
  TraceValue("%lu",dhtKeyCount(pyhash));
  TraceValue("%lu",targetKeyCount);
  TraceEOL();

  while (true)
  {
    dhtElement const *he;

#if defined(TESTHASH)
    unsigned long nrElementsVisitedInIteration = 0;
    printf("starting iteration: minimalElementValueAfterCompression: %u\n",
           minimalElementValueAfterCompression);
    fflush(stdout);
#endif  /* TESTHASH */

#if defined(TESTHASH)
    for (he = dhtGetFirstElement(pyhash);
         he!=0;
         he = dhtGetNextElement(pyhash))
      printf("%u\n",value_of_data(he));
#endif  /* TESTHASH */

    for (he = dhtGetFirstElement(pyhash);
         he!=0;
         he = dhtGetNextElement(pyhash))
    {
      hashElement_union_t const * const hue = (hashElement_union_t const *)he;
      if (value_of_data(hue)<minimalElementValueAfterCompression)
      {
#if defined(TESTHASH)
        ++nrElementsRemovedInCompression;
        ++nrElementsRemovedInAllCompressions;
#endif  /* TESTHASH */

        dhtRemoveElement(pyhash, hue->d.Key);

#if defined(TESTHASH)
        if (nrElementsRemovedInCompression + dhtKeyCount(pyhash) != nrElementsAtStartOfCompression)
        {
          printf("dhtRemove failed on %lu-th element of iteration %lu. "
                 "This was the %lu-th call to dhtRemoveElement.\n"
                 "nrElementsRemovedInCompression=%lu "
                 "dhtKeyCount=%lu "
                 "nrElementsAtStartOfCompression=%lu\n",
                 nrElementsVisitedInIteration,
                 nrIterationsInCompression,
                 nrElementsRemovedInAllCompressions,
                 nrElementsRemovedInCompression,
                 dhtKeyCount(pyhash),
                 nrElementsAtStartOfCompression);
          exit(1);
        }
#endif  /* TESTHASH */
      }
#if defined(TESTHASH)
      ++nrElementsVisitedInIteration;
#endif  /* TESTHASH */
    }

#if defined(TESTHASH)
    ++nrIterationsInCompression;
    assert(nrElementsAtStartOfCompression>=nrElementsVisitedInIteration);
    printf("iteration=%lu, nrElementsRemovedInCompression:%lu, missed:%lu\n",
           nrIterationsInCompression,
           nrElementsRemovedInCompression,
           nrElementsAtStartOfCompression-nrElementsVisitedInIteration);
    {
      unsigned int i;
      unsigned int counter[16];
      unsigned long const KeyCount = dhtKeyCount(pyhash);
      dhtBucketStat(pyhash,counter,16);
      for (i=0; i<16; ++i)
        printf("%lu %u %u\n", KeyCount, i+1, counter[i]);
      putchar('\n');
      if (nrIterationsInCompression>9)
        printf("nrIterationsInCompression > 9 after %lu-th call to  dhtRemoveElement\n",
               nrElementsRemovedInAllCompressions);
      dhtDebug = nrIterationsInCompression==9;
    }
    fflush(stdout);
#endif  /* TESTHASH */

    if (dhtKeyCount(pyhash)<=targetKeyCount)
      break;
    else
      ++minimalElementValueAfterCompression;
  }
#if defined(TESTHASH)
  printf("%lu;",dhtKeyCount(pyhash));
#if defined(HASHRATE)
  printf(" usage: %lu", use_pos);
  printf(" / %lu", use_all);
  printf(" = %.1f%%", (100.0 * use_pos) / use_all);
#endif
#if defined(FREEMAP) && defined(FXF)
  PrintFreeMap(stdout);
#endif /*FREEMAP*/
  putchar('\n');
#if defined(FXF)
  puts("\n after compression:");
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
  output_plaintext_print_time("  ","");
  Message(IncrementHashRateLevel,HashRateLevel);
  HashStats(0, "\n");
}

void DecHashRateLevel(void)
{
  if (HashRateLevel>0)
    --HashRateLevel;
  output_plaintext_print_time("  ","");
  Message(DecrementHashRateLevel,HashRateLevel);
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

  if (level<=HashRateLevel)
  {
    fputs("  ",stdout);
    pos= dhtKeyCount(pyhash);
    Message2(stdout,HashedPositions,pos);
    if (use_all > 0)
    {
      if (use_all < 10000)
        printf(", %lu/%lu = %lu%%",
               use_pos, use_all, (use_pos*100) / use_all);
      else
        printf(", %lu/%lu = %lu%%",
               use_pos, use_all, use_pos / (use_all/100));
    }
    else
      fputs(" -",stdout);
    if (HashRateLevel > 3)
    {
      unsigned long msec;
      unsigned long Seconds;
      StopTimer(&Seconds,&msec);
      if (Seconds > 0)
        printf(", %lu pos/s", use_all/Seconds);
    }
    if (trailer)
      StdString2(stdout,trailer);
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
    stip_length_type const length = SLICE_U(si).branch.length;
    switch (SLICE_TYPE(si))
    {
      case STAttackHashed:
        result += (length-slack_length)/2;
        break;

      case STHelpHashed:
        result += (length-slack_length+1)/2;
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

static void ProofSmallEncodePiece(byte **bp,
                                  int row, int col,
                                  piece_walk_type p, Flags flags,
                                  boolean *even)
{
  Side const side =  TSTFLAG(flags,White) ? White : Black;
  byte encoded = p;
  assert(!is_piece_neutral(flags));
  if (side==Black)
    encoded |= 1 << black_bit;
  assert(p < 1 << black_bit);
  if (*even)
  {
    **bp += encoded<<(CHAR_BIT/2);
    ++*bp;
  }
  else
    **bp = encoded;
  *even = !*even;
}

static void ProofLargeEncodePiece(byte **bp,
                                  unsigned int row, unsigned int col,
                                  piece_walk_type p, Flags flags)
{
  **bp = p;
  ++*bp;

  **bp = flags&COLOURFLAGS;
  ++*bp;
}

static void ProofEncode(stip_length_type min_length, stip_length_type validity_value)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *position = hb->cmv.Data;
  byte *bp = position+nr_rows_on_board;

  /* clear the bits for storing the position of pieces */
  memset(position, 0, nr_rows_on_board);

  {
    boolean even = false;
    square a_square= square_a1;
    unsigned int row;
    for (row = 0; row<nr_rows_on_board; ++row, a_square += onerow)
    {
      square curr_square = a_square;
      unsigned int col;
      for (col = 0; col<nr_files_on_board; ++col, curr_square += dir_right)
      {
        piece_walk_type const p = get_walk_of_piece_on_square(curr_square);
        if (p!=Empty)
        {
          Flags const flags = being_solved.spec[curr_square];
          if (piece_walk_may_exist_fairy || is_piece_neutral(some_pieces_flags))
            ProofLargeEncodePiece(&bp,row,col,p,flags);
          else
            ProofSmallEncodePiece(&bp,row,col,p,flags,&even);
          position[row] |= BIT(col);
        }
      }
    }

    if (even)
      ++bp;
  }

  {
    underworld_index_type gi;
    for (gi = 0; gi<nr_ghosts; ++gi)
    {
      square s = (underworld[gi].on
                  - nr_of_slack_rows_below_board*onerow
                  - nr_of_slack_files_left_of_board);
      unsigned int const row = s/onerow;
      unsigned int const col = s%onerow;
      bp = SmallEncodePiece(bp,
                            row,col,
                            underworld[gi].walk,underworld[gi].flags);
    }
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp,min_length,validity_value);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);
}

static unsigned int TellCommonEncodePosLeng(unsigned int len,
                                            unsigned int nbr_p)
{
  len += 2; /* Castling_Flag, validity_value */

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

  if (CondFlag[blacksynchron] || CondFlag[whitesynchron]
      || CondFlag[blackantisynchron] || CondFlag[whiteantisynchron])
    len++;

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;
    for (imi_idx = 0; imi_idx<being_solved.number_of_imitators; imi_idx++)
      len++;

    /* coding of no. of imitators and average of one
       imitator-promotion assumed.
    */
    len+=2;
  }

  if (circe_variant.relevant_capture==circe_relevant_capture_lastmove)
    /*
    ** only one out of three positions with a capture
    ** assumed.
    */
    len++;

  if (OptFlag[nontrivial])
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
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp= boardnum; *bnp; bnp++)
    if (!is_square_empty(*bnp))
    {
      len += bytes_per_piece;
      nbr_p++;  /* count no. of pieces and holes */
    }

  if (CondFlag[BGL])
    len += sizeof BGL_values[White] + sizeof BGL_values[Black];

  len += nr_ghosts*bytes_per_piece;

  result = TellCommonEncodePosLeng(len, nbr_p);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
} /* TellLargeEncodePosLeng */

static unsigned int TellSmallEncodePosLeng(void)
{
  square const *bnp;
  unsigned int nbr_p = 0;
  unsigned int len = 0;
  unsigned int result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (bnp= boardnum; *bnp; bnp++)
  {
    /* piece    p;
    ** Flags    pspec;
    */
    if (!is_square_empty(*bnp))
    {
      len += 1 + bytes_per_piece;
      nbr_p++;            /* count no. of pieces and holes */
    }
  }

  len += nr_ghosts*bytes_per_piece;

  result = TellCommonEncodePosLeng(len, nbr_p);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
} /* TellSmallEncodePosLeng */

byte *CommonEncode(byte *bp,
                   stip_length_type min_length,
                   stip_length_type validity_value)
{
  if (CondFlag[messigny])
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    if (move_effect_journal[movement].type==move_effect_piece_exchange
        && move_effect_journal[movement].reason==move_effect_reason_messigny_exchange)
    {
      *bp++ = (byte)(move_effect_journal[movement].u.piece_exchange.to - square_a1);
      *bp++ = (byte)(move_effect_journal[movement].u.piece_exchange.from - square_a1);
    }
    else
      *bp++ = (byte)UCHAR_MAX;
  }

  if (CondFlag[duellist])
  {
    *bp++ = (byte)(duellists[White] - square_a1);
    *bp++ = (byte)(duellists[Black] - square_a1);
  }

  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue])
  {
    square const sq_departure = move_effect_journal_get_departure_square(nbply);
    if (sq_departure==initsquare)
      *bp++ = (byte)UCHAR_MAX;
    else
      *bp++ = (byte)(sq_departure-square_a1);
  }

  if (CondFlag[blacksynchron] || CondFlag[whitesynchron]
      || CondFlag[blackantisynchron] || CondFlag[whiteantisynchron])
  {
    square const sq_departure = move_effect_journal_get_departure_square(nbply);
    if (sq_departure==initsquare)
      *bp++ = (byte)UCHAR_MAX;
    else
    {
      move_effect_journal_index_type const base = move_effect_journal_base[nbply];
      move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
      square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;
      enum { nr_squares = nr_rows_on_board*nr_files_on_board };
      *bp++= (byte)(sq_num(sq_departure)-sq_num(sq_arrival)+nr_squares);
    }
  }

  if (CondFlag[imitators])
  {
    unsigned int imi_idx;

    /* The number of imitators has to be coded too to avoid
     * ambiguities.
     */
    *bp++ = (byte)being_solved.number_of_imitators;
    for (imi_idx = 0; imi_idx<being_solved.number_of_imitators; imi_idx++)
      *bp++ = (byte)(being_solved.isquare[imi_idx]-square_a1);
  }

  if (OptFlag[nontrivial])
    *bp++ = (byte)(max_nr_nontrivial);

  if (circe_variant.relevant_capture==circe_relevant_capture_lastmove)
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    if (move_effect_journal[capture].type==move_effect_piece_removal)
    {
      /* a piece has been captured and can be reborn */
      square const from = move_effect_journal[capture].u.piece_removal.on;
      piece_walk_type const removed = move_effect_journal[capture].u.piece_removal.walk;
      Flags const removedspec = move_effect_journal[capture].u.piece_removal.flags;

      *bp++ = (byte)(from-square_a1);
      if (one_byte_hash)
        *bp++ = (byte)(removedspec) + ((byte)(piece_nbr[removed]) << (CHAR_BIT/2));
      else
      {
        *bp++ = removed;
        *bp++ = (byte)(removedspec>>CHAR_BIT);
        *bp++ = (byte)(removedspec&ByteMask);
      }
    }
    else
      *bp++ = (byte)0;
  }

  assert(validity_value<=(1<<CHAR_BIT));
  if (min_length>slack_length+1)
    *bp++ = (byte)(validity_value);

  {
    unsigned int i;

    for (i = en_passant_top[nbply-1]+1; i<=en_passant_top[nbply]; ++i)
      *bp++ = (byte)(en_passant_multistep_over[i] - square_a1);
  }

  *bp++ = being_solved.castling_rights;     /* Castling_Flag */

  if (CondFlag[BGL]) {
    memcpy(bp, &BGL_values[White], sizeof BGL_values[White]);
    bp += sizeof BGL_values[White];
    memcpy(bp, &BGL_values[Black], sizeof BGL_values[Black]);
    bp += sizeof BGL_values[Black];
  }

  if (CondFlag[disparate])
  {
    move_effect_journal_index_type const top = move_effect_journal_base[nbply];
    move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
    *bp++ = (byte)move_effect_journal[movement].u.piece_movement.moving;
    *bp++ = trait[nbply];
  }

  return bp;
} /* CommonEncode */

static byte *LargeEncodePiece(byte *bp, byte *position,
                              int row, int col,
                              piece_walk_type pienam, Flags pspec)
{
  if (one_byte_hash)
    *bp++ = (byte)pspec + ((byte)piece_nbr[pienam] << (CHAR_BIT/2));
  else
  {
    unsigned int i;
    *bp++ = pienam;
    for (i = 0; i<bytes_per_spec; i++)
      *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
  }

  position[row] |= BIT(col);

  return bp;
}

static void LargeEncode(stip_length_type min_length,
                        stip_length_type validity_value)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *position = hb->cmv.Data;
  byte *bp = position+nr_rows_on_board;
  int row, col;
  square a_square = square_a1;
  underworld_index_type gi;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* clear the bits for storing the position of pieces */
  memset(position,0,nr_rows_on_board);

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow)
  {
    square curr_square = a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right)
    {
      piece_walk_type const p = get_walk_of_piece_on_square(curr_square);
      if (p!=Empty)
        bp = LargeEncodePiece(bp,position,row,col,p,being_solved.spec[curr_square]);
    }
  }

  for (gi = 0; gi<nr_ghosts; ++gi)
  {
    square s = (underworld[gi].on
                - nr_of_slack_rows_below_board*onerow
                - nr_of_slack_files_left_of_board);
    row = s/onerow;
    col = s%onerow;
    bp = LargeEncodePiece(bp,position,
                          row,col,
                          underworld[gi].walk,underworld[gi].flags);
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp,min_length,validity_value);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* LargeEncode */

byte *SmallEncodePiece(byte *bp,
                       int row, int col,
                       piece_walk_type pienam, Flags pspec)
{
  *bp++= (byte)((row<<(CHAR_BIT/2))+col);
  if (one_byte_hash)
    *bp++ = (byte)pspec + ((byte)piece_nbr[pienam] << (CHAR_BIT/2));
  else
  {
    unsigned int i;
    *bp++ = pienam;
    for (i = 0; i<bytes_per_spec; i++)
      *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
  }

  return bp;
}

static void SmallEncode(stip_length_type min_length,
                        stip_length_type validity_value)
{
  HashBuffer *hb = &hashBuffers[nbply];
  byte *bp = hb->cmv.Data;
  square a_square = square_a1;
  int row;
  int col;
  underworld_index_type gi;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (row=0; row<nr_rows_on_board; row++, a_square += onerow)
  {
    square curr_square= a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square += dir_right)
    {
      piece_walk_type const p = get_walk_of_piece_on_square(curr_square);
      if (p!=Empty)
        bp = SmallEncodePiece(bp,row,col,p,being_solved.spec[curr_square]);
    }
  }

  for (gi = 0; gi<nr_ghosts; ++gi)
  {
    square s = (underworld[gi].on
                - nr_of_slack_rows_below_board*onerow
                - nr_of_slack_files_left_of_board);
    row = s/onerow;
    col = s%onerow;
    bp = SmallEncodePiece(bp,
                          row,col,
                          underworld[gi].walk,underworld[gi].flags);
  }

  /* Now the rest of the party */
  bp = CommonEncode(bp,min_length,validity_value);

  assert(bp-hb->cmv.Data<=UCHAR_MAX);
  hb->cmv.Leng = (unsigned char)(bp-hb->cmv.Data);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
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
    switch (SLICE_TYPE(si))
    {
      case STAttackHashed:
      {
        stip_length_type const length = SLICE_U(si).branch.length;
        stip_length_type const min_length = SLICE_U(si).branch.min_length;
        set_value_attack_nosuccess(hue,si,0);
        set_value_attack_success(hue,si,(length-min_length+1)/2);
        break;
      }

      case STHelpHashed:
        set_value_help(hue,si,0);
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
#if defined(FXF)
      fxfReset();
#endif
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
    puts("Sorry, cannot enter more hashelements "
         "despite compression");
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
  size_t const one_kilo = 1<<10;
  while (fxfInit(nr_kilos*one_kilo)==-1)
    /* we didn't get hashmemory ... */
    nr_kilos /= 2;
  ifTESTHASH(fxfInfo(stdout));
#endif /*FXF*/

  hashtable_kilos = nr_kilos;
  return nr_kilos;
}

static void proof_goal_found(slice_index si, stip_structure_traversal *st)
{
  boolean * const result = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  *result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_proofgame(slice_index si)
{
  boolean result = false;
  stip_structure_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&result);
  if (SLICE_TYPE(si)==STIntelligentFilter)
    st.context = stip_traversal_context_help;
  stip_structure_traversal_override_single(&st,
                                           STGoalProofgameReachedTester,
                                           &proof_goal_found);
  stip_structure_traversal_override_single(&st,
                                           STGoalAToBReachedTester,
                                           &proof_goal_found);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the hash table has been successfully allocated
 * @return true iff the hashtable has been allocated
 */
boolean is_hashtable_allocated(void)
{
  return fxfInitialised();
}

/* Initialise the hashing machinery for the current stipulation
 * @param si identifies the root slice of the stipulation
 */
static void inithash(slice_index si)
{
  unsigned int j;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(pyhash==0);

  ifTESTHASH(puts("calling inithash"));

#if defined(__unix) && defined(TESTHASH)
  OldBreak= sbrk(0);
#endif /*__unix,TESTHASH*/

  minimalElementValueAfterCompression = 2;

  init_slice_properties(si);

  template_element.d.Data = 0;
  init_elements(&template_element);

  is_table_uncompressed = true;     /* V3.60  TLi */

  dhtRegisterValue(dhtBCMemValue,0,&dhtBCMemoryProcs);
  dhtRegisterValue(dhtSimpleValue,0,&dhtSimpleProcs);

  ifHASHRATE(use_pos = use_all = 0);

  /* check whether a piece can be coded in a single byte */
  j = 0;

  for (piece_walk_type i = nr_piece_walks; i != Empty;)
  {
    --i;
    if (piece_walk_may_exist[i])
      piece_nbr[i] = j++;
  }

  if (CondFlag[haanerchess])
    piece_nbr[Invalid]= j++;

  one_byte_hash = j<(1<<(CHAR_BIT/2)) && some_pieces_flags<(1<<(CHAR_BIT/2));

  bytes_per_spec= 1;
  if ((some_pieces_flags >> CHAR_BIT) != 0)
    bytes_per_spec++;
  if ((some_pieces_flags >> 2*CHAR_BIT) != 0)
    bytes_per_spec++;

  bytes_per_piece= one_byte_hash ? 1 : 1+bytes_per_spec;

  if (is_proofgame(si))
  {
    encode = ProofEncode;
    if (hashtable_kilos>0 && hash_max_number_storable_positions==0)
      hash_max_number_storable_positions= hashtable_kilos/(24+sizeof(char *)+1);
  }
  else
  {
    unsigned int const Small = TellSmallEncodePosLeng();
    unsigned int const Large = TellLargeEncodePosLeng();
    if (Small<=Large)
    {
      encode = SmallEncode;
      if (hashtable_kilos>0 && hash_max_number_storable_positions==0)
        hash_max_number_storable_positions= hashtable_kilos/(Small+sizeof(char *)+1);
    }
    else
    {
      encode = LargeEncode;
      if (hashtable_kilos>0 && hash_max_number_storable_positions==0)
        hash_max_number_storable_positions= hashtable_kilos/(Large+sizeof(char *)+1);
    }
  }

#if defined(FXF)
  ifTESTHASH(printf("MaxPositions: %lu\n", hash_max_number_storable_positions));
  assert(hashtable_kilos/1024<UINT_MAX);
  ifTESTHASH(printf("hashtable_kilos:    %7u KB\n",
                    (unsigned int)(hashtable_kilos/1024)));
#else
  ifTESTHASH(
      printf("room for up to %lu positions in hash table\n", hash_max_number_storable_positions));
#endif /*FXF*/

#if defined(TESTHASH) && defined(FXF)
  fxfInfo(stdout);
#endif /*TESTHASH,FXF*/

#if defined(FXF)
  fxfReset();
#endif

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void openhash(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(pyhash==0);
  pyhash = dhtCreate(dhtBCMemValue,dhtCopy,dhtSimpleValue,dhtNoCopy);
  assert(pyhash!=0);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
} /* inithash */

/* Uninitialise the hashing machinery
 */
static void closehash(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(pyhash!=0);

#if defined(TESTHASH)
  puts("calling closehash");

#if defined(HASHRATE)
  printf("%lu enquiries out of %lu successful. ",use_pos,use_all);
  if (use_all)
    printf("Makes %.1f%%\n",(100.0 * use_pos) / use_all);
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
      printf("Memory for hash-table: %lu, "
             "gives %lu.%02lu bytes per position\n",
             HashMem, BytePerPos/100, BytePerPos%100);
    }
    else
      puts("Nothing in hashtable");
  }
#endif /*__unix*/
#endif /*TESTHASH*/

  dhtDestroy(pyhash);
  pyhash = 0;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin a tester slice off a STAttackHashedTester slice
 * @param base_slice identifies the STAttackHashedTester slice
 * @return id of allocated slice
 */
static void spin_off_tester_attack(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_TESTER(si) = alloc_pipe(STAttackHashedTester);
  SLICE_U(SLICE_TESTER(si)).derived_pipe.base = si;
  stip_traverse_structure_children_pipe(si,st);
  pipe_append(SLICE_TESTER(SLICE_PREV(si)),SLICE_TESTER(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin a tester slice off a STHelpHashed slice
 * @param base_slice identifies the STHelpHashed slice
 * @return id of allocated slice
 */
static void spin_off_tester_help(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  SLICE_TESTER(si) = alloc_pipe(STHelpHashedTester);
  SLICE_U(SLICE_TESTER(si)).derived_pipe.base = si;
  stip_traverse_structure_children_pipe(si,st);
  pipe_append(SLICE_TESTER(SLICE_PREV(si)),SLICE_TESTER(si));

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    slice_index previous_move_slice;
    boolean opener_inserted;
} insertion_state_type;

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 */
static void insert_hash_element_attack(slice_index si,
                                       stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;
  stip_length_type const length = SLICE_U(si).branch.length;
  stip_length_type const min_length = SLICE_U(si).branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->previous_move_slice!=no_slice && length>slack_length)
  {
    slice_index const prototype = alloc_branch(STAttackHashed,length,min_length);
    attack_branch_insert_slices(si,&prototype,1);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Traverse a slice while inserting hash elements
 * @param si identifies slice
 * @param st address of structure holding status of traversal
 */
static void insert_hash_element_help(slice_index si,
                                     stip_structure_traversal *st)
{
  insertion_state_type const * const state = st->param;
  stip_length_type const length = SLICE_U(si).branch.length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->previous_move_slice!=no_slice && length>slack_length)
  {
    stip_length_type min_length = SLICE_U(si).branch.min_length;
    if (min_length<slack_length)
      min_length += 2;

    {
      slice_index const prototype = alloc_branch(STHelpHashed,length,min_length);
      help_branch_insert_slices(si,&prototype,1);
    }
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_move(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;
  slice_index const save_previous_move_slice = state->previous_move_slice;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  state->previous_move_slice = si;
  stip_traverse_structure_children_pipe(si,st);
  state->previous_move_slice = save_previous_move_slice;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_hash_opener(slice_index si, stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    slice_index const prototype = alloc_pipe(STHashOpener);
    help_branch_insert_slices(si,&prototype,1);
  }

  state->opener_inserted = true;

  stip_traverse_structure_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const hash_element_inserters[] =
{
  { STReadyForAttack,    &insert_hash_element_attack },
  { STReadyForHelpMove,  &insert_hash_element_help   },
  { STMove,              &remember_move              },
  { STAttackHashed,      &spin_off_tester_attack     },
  { STHelpHashed,        &spin_off_tester_help       },
  { STIntelligentFilter, &insert_hash_opener         }
};

enum
{
  nr_hash_element_inserters
  = sizeof hash_element_inserters / sizeof hash_element_inserters[0]
};

/* Instrument stipulation with hashing slices
 * @param si identifies slice where to start
 */
void solving_insert_hashing(slice_index si)
{
  stip_structure_traversal st;
  insertion_state_type state = { no_slice, false };

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&state);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_conditional_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_contextual(&st,
                                                  slice_contextual_testing_pipe,
                                                  &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    hash_element_inserters,
                                    nr_hash_element_inserters);
  stip_traverse_structure(si,&st);

  if (!state.opener_inserted)
  {
    slice_index const opener = alloc_pipe(STHashOpener);
    pipe_append(si,opener);
    SLICE_TESTER(opener) = alloc_proxy_slice();
    pipe_append(SLICE_TESTER(si),SLICE_TESTER(opener));
  }

  inithash(si);

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
void attack_hashed_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((SLICE_U(si).branch.length-solve_nr_remaining)%2==0);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Remember that the current position does not have a solution in a
 * number of half-moves
 * @param si index of slice where the current position was reached
 * @param n number of half-moves
 */
static void addtohash_battle_nosuccess(slice_index si,
                                       stip_length_type n,
                                       stip_length_type min_length_adjusted)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = (n+1-min_length_adjusted)/2;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",min_length_adjusted);
  TraceFunctionParamListEnd();

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_attack_nosuccess(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_attack_nosuccess(hue,si)<val)
      set_value_attack_nosuccess(hue,si,val);
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
static void addtohash_battle_success(slice_index si,
                                     stip_length_type n,
                                     stip_length_type min_length_adjusted)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  hash_value_type const val = (n+1-min_length_adjusted)/2 - 1;
  dhtElement *he;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",min_length_adjusted);
  TraceFunctionParamListEnd();

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
  {
    hashElement_union_t * const
        hue = (hashElement_union_t *)allocDHTelement(hb);
    set_value_attack_success(hue,si,val);
  }
  else
  {
    hashElement_union_t * const hue = (hashElement_union_t *)he;
    if (get_value_attack_success(hue,si)>val)
      set_value_attack_success(hue,si,val);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
}

static
stip_length_type delegate_can_attack_in_n(slice_index si,
                                          stip_length_type min_length_adjusted)
{
  stip_length_type result;
  slice_index const base = SLICE_U(si).derived_pipe.base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",min_length_adjusted);
  TraceFunctionParamListEnd();

  pipe_solve_delegate(si);

  result = solve_result;

  if (result<=MOVE_HAS_SOLVED_LENGTH())
    addtohash_battle_success(base,result,min_length_adjusted);
  else
    addtohash_battle_nosuccess(base,solve_nr_remaining,min_length_adjusted);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
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
void attack_hashed_tester_solve(slice_index si)
{
  dhtElement const *he;
  slice_index const base = SLICE_U(si).derived_pipe.base;
  stip_length_type const min_length = SLICE_U(base).branch.min_length;
  stip_length_type const played = SLICE_U(base).branch.length-solve_nr_remaining;
  stip_length_type const min_length_adjusted = (min_length<played+slack_length-1
                                                ? slack_length-(min_length-slack_length)%2
                                                : min_length-played);
  stip_length_type const validity_value = min_length_adjusted/2+1;
  stip_length_type const save_max_unsolvable = max_unsolvable;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert((solve_nr_remaining-SLICE_U(base).branch.length)%2==0);

  (*encode)(min_length,validity_value);

  he = dhtLookupElement(pyhash,&hashBuffers[nbply]);
  if (he==dhtNilElement)
    solve_result = delegate_can_attack_in_n(si,min_length_adjusted);
  else
  {
    hashElement_union_t const * const hue = (hashElement_union_t const *)he;
    stip_length_type const parity = (solve_nr_remaining-min_length_adjusted)%2;

    /* It is more likely that a position has no solution. */
    /* Therefore let's check for "no solution" first.  TLi */
    hash_value_type const val_nosuccess = get_value_attack_nosuccess(hue,base);
    stip_length_type const n_nosuccess = 2*val_nosuccess + min_length_adjusted-parity;
    if (n_nosuccess>=MOVE_HAS_SOLVED_LENGTH())
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
    else
    {
      hash_value_type const val_success = get_value_attack_success(hue,base);
      stip_length_type const n_success = 2*val_success + min_length_adjusted+2-parity;
      if (n_success<=MOVE_HAS_SOLVED_LENGTH())
        solve_result = n_success;
      else
      {
        if (max_unsolvable<n_nosuccess)
        {
          max_unsolvable = n_nosuccess;
          TraceValue("->%u",max_unsolvable);
          TraceEOL();
        }
        solve_result = delegate_can_attack_in_n(si,min_length_adjusted);
      }
    }
  }

  max_unsolvable = save_max_unsolvable;
  TraceValue("->%u",max_unsolvable);
  TraceEOL();

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
static boolean inhash_help(slice_index si)
{
  boolean result;
  HashBuffer *hb = &hashBuffers[nbply];
  dhtElement const *he;
  stip_length_type const validity_value = (solve_nr_remaining-1)/2+1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* In help play, we encode all positions as if the stipulation were exact.
   * This is necessary to avoid ruling out some solutions with intermediate
   * positions that are solvable in, say, m moves but not in m+1. This only
   * makes a difference in problems with short cooks, but not printing all
   * solutions is confusing and if we measure, the price that we are paying
   * is smaller than one might think. TM
   */
  (*encode)(solve_nr_remaining,validity_value);

  ifHASHRATE(use_all++);

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
    result = false;
  else if (get_value_help((hashElement_union_t const *)he,si)==1)
  {
    ifHASHRATE(use_pos++);
    result = true;
  }
  else
    result = false;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Remember that the current position does not have a solution in a
 * number of half-moves
 * @param si index of slice where the current position was reached
 */
static void addtohash_help(slice_index si)
{
  HashBuffer const * const hb = &hashBuffers[nbply];
  dhtElement *he;
  hashElement_union_t * hue;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  he = dhtLookupElement(pyhash,hb);
  if (he==dhtNilElement)
    hue = (hashElement_union_t *)allocDHTelement(hb);
  else
    hue = (hashElement_union_t *)he;

  set_value_help(hue,si,1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
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
void help_hashed_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  if (is_table_uncompressed || solve_nr_remaining>next_move_has_solution)
  {
    if (inhash_help(si))
      solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
    else
    {
      if (SLICE_U(si).branch.min_length>slack_length+1)
      {
        SLICE_U(si).branch.min_length -= 2;
        pipe_solve_delegate(si);
        SLICE_U(si).branch.min_length += 2;
      }
      else
        pipe_solve_delegate(si);

      if (solve_result==MOVE_HAS_NOT_SOLVED_LENGTH())
        addtohash_help(si);
    }
  }
  else
    pipe_solve_delegate(si);

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
void help_hashed_tester_solve(slice_index si)
{
  slice_index const base = SLICE_U(si).derived_pipe.base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(solve_nr_remaining>=next_move_has_solution);

  if (inhash_help(base))
    solve_result = MOVE_HAS_NOT_SOLVED_LENGTH();
  else
  {
    if (SLICE_U(base).branch.min_length>slack_length+1)
    {
      SLICE_U(base).branch.min_length -= 2;
      pipe_solve_delegate(si);
      SLICE_U(base).branch.min_length += 2;
    }
    else
      pipe_solve_delegate(si);

    if (solve_result>MOVE_HAS_SOLVED_LENGTH())
      addtohash_help(base);
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
void hash_opener_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  openhash();
  pipe_solve_delegate(si);

  closehash();

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
  assert(nr_piece_flags<=2*CHAR_BIT);
}
