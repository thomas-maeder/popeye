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
 ** 2005/02/01 TLi  in d_composite_does_attacker_win and invref exchanged the inquiry into the hash
 **                 table for "white can mate" and "white cannot mate" because
 **                 it is more likely that a position has no solution
 **                 This yields an incredible speedup of .5-1%  *fg*
 **
 ** 2006/06/30 SE   New condition: BGL (invented P.Petkov)
 **
 ** 2008/02/10 SE   New condition: Cheameleon Pursuit (invented? : L.Grolman)  
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
#include "pycompos.h"
#include "platform/maxtime.h"
#include "trace.h"

static struct dht *pyhash;

static char    piece_nbr[PieceCount];
static boolean one_byte_hash;
static unsigned int bytes_per_spec;
static unsigned int bytes_per_piece;

unsigned long int compression_counter;


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

void (*encode)(HashBuffer*);

typedef unsigned int data_type;

typedef struct
{
	dhtValue Key;
    data_type data;
} element_t;


/* Hashing properties of stipulation slices
 */

typedef struct
{
    unsigned int size;
    unsigned int value_size;

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
            unsigned int offsetNoSuccOdd;
            unsigned int maskNoSuccOdd;
            unsigned int offsetNoSuccEven;
            unsigned int maskNoSuccEven;
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

/* Initialize the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a direct slice.
 * @param si root slice of subtree
 * @param length number of attacker's moves of help slice
 * @param nr_bits_left number of bits left over by slices already init
 * @note this is an indirectly recursive function
 */
static void init_slice_properties_direct(slice_index si,
                                         unsigned int length,
                                         size_t *nr_bits_left)
{
  unsigned int const size = bit_width(length);
  data_type const mask = (1<<size)-1;

  slice_properties[si].size = size;
  slice_properties[si].value_size = size;

  assert(*nr_bits_left>=size);
  *nr_bits_left -= size;
  slice_properties[si].u.d.offsetNoSucc = *nr_bits_left;
  slice_properties[si].u.d.maskNoSucc = mask << *nr_bits_left;

  assert(*nr_bits_left>=size);
  *nr_bits_left -= size;
  slice_properties[si].u.d.offsetSucc = *nr_bits_left;
  slice_properties[si].u.d.maskSucc = mask << *nr_bits_left;
}

/* Initialize the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a help slice.
 * @param si root slice of subtree
 * @param length number of half moves of help slice
 * @param nr_bits_left number of bits left over by slices already init
 * @note this is an indirectly recursive function
 */
static void init_slice_properties_help(slice_index si,
                                       unsigned int length,
                                       size_t *nr_bits_left)
{
  unsigned int const size = bit_width((length+1)/2);
  data_type const mask = (1<<size)-1;

  slice_properties[si].size = size;
  slice_properties[si].value_size = size+1;

  assert(*nr_bits_left>=size);
  *nr_bits_left -= size;
  slice_properties[si].u.h.offsetNoSuccOdd = *nr_bits_left;
  slice_properties[si].u.h.maskNoSuccOdd = mask << *nr_bits_left;

  assert(*nr_bits_left>=size);
  *nr_bits_left -= size;
  slice_properties[si].u.h.offsetNoSuccEven = *nr_bits_left;
  slice_properties[si].u.h.maskNoSuccEven = mask << *nr_bits_left;
}

/* Initialize the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a series slice.
 * @param si root slice of subtree
 * @param length number of half moves of series slice
 * @param nr_bits_left number of bits left over by slices already init
 * @note this is an indirectly recursive function
 */
static void init_slice_properties_series(slice_index si,
                                         unsigned int length,
                                         size_t *nr_bits_left)
{
  unsigned int const size = bit_width(length);
  data_type const mask = (1<<size)-1;

  slice_properties[si].size = size;
  slice_properties[si].value_size = size;

  assert(*nr_bits_left>=size);
  *nr_bits_left -= size;
  slice_properties[si].u.s.offsetNoSucc = *nr_bits_left;
  slice_properties[si].u.s.maskNoSucc = mask << *nr_bits_left;
}

static void init_slice_properties_recursive(slice_index si,
                                            size_t *nr_bits_left);

/* Initialize the slice_properties array according to a subtree of the
 * current stipulation slices whose root is a composite slice.
 * @param si root slice of subtree
 * @param nr_bits_left number of bits left over by slices already init
 * @note this is an indirectly recursive function
 */
static void init_slice_properties_composite(slice_index si,
                                            size_t *nr_bits_left)
{
  unsigned int const length = slices[si].u.composite.length;
  switch (slices[si].u.composite.play)
  {
    case PDirect:
     init_slice_properties_direct(si,
                                  length-slack_length_direct,
                                  nr_bits_left);
      break;

    case PHelp:
      init_slice_properties_help(si,
                                 length-slack_length_help,
                                 nr_bits_left);
      break;

    case PSeries:
      init_slice_properties_series(si,
                                   length-slack_length_series,
                                   nr_bits_left);
      break;

    default:
      assert(0);
      break;
  }

  switch (slices[si].type)
  {
    case STLeaf:
      assert(0);

    case STReciprocal:
    case STQuodlibet:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;

      init_slice_properties_recursive(op1,nr_bits_left);
      init_slice_properties_recursive(op2,nr_bits_left);

      /* both operand slices must have the same value_size, or the
       * shorter one will dominate the longer one */
      if (slice_properties[op1].value_size>slice_properties[op2].value_size)
        slice_properties[op2].value_size = slice_properties[op1].value_size;
      else
        slice_properties[op1].value_size = slice_properties[op2].value_size;

      break;
    }

    case STSequence:
    {
      slice_index const op1 = slices[si].u.composite.op1;
      init_slice_properties_recursive(op1,nr_bits_left);
      break;
    }
  }
}

/* Initialize the slice_properties array according to a leaf slice
 * @param leaf leaf slice
 * @param nr_bits_left number of bits left over by slices already init
 */
static void init_slice_properties_leaf(slice_index leaf,
                                       size_t *nr_bits_left)
{
  switch (slices[leaf].u.leaf.end)
  {
    case EHelp:
      init_slice_properties_help(leaf,2,nr_bits_left);
      break;

    case EDirect:
    case ESelf:
    case EReflex:
    case ESemireflex:
      init_slice_properties_direct(leaf,1,nr_bits_left);
      break;

    default:
      assert(0);
      break;
  }
}

/* Initialize the slice_properties array according to a subtree of the
 * current stipulation slices.
 * @param si root slice of subtree
 * @param nr_bits_left number of bits left over by slices already init
 * @return number of bits left over for subsequent slices
 */
static void init_slice_properties_recursive(slice_index si,
                                            size_t *nr_bits_left)
{
  if (slices[si].type==STLeaf)
    init_slice_properties_leaf(si,nr_bits_left);
  else
    init_slice_properties_composite(si,nr_bits_left);
}

/* Initialize the slice_properties array according to the current
 * stipulation slices.
 */
static void init_slice_properties(void)
{
  slice_index const si = 0;
  size_t nr_bits_left = sizeof(data_type)*CHAR_BIT;
  init_slice_properties_recursive(si,&nr_bits_left);
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
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",val);
  TraceValue("%d ",slice_properties[si].size);
  TraceValue("%d ",offset);
  TraceValue("%08x ",mask);
  TraceValue("%p ",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceText("\n");
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
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",val);
  TraceValue("%d ",slice_properties[si].size);
  TraceValue("%d ",offset);
  TraceValue("%08x ",mask);
  TraceValue("%p ",&e->data);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

static void set_value_help_odd(dhtElement *he,
                               slice_index si,
                               hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSuccOdd;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.h.maskNoSuccOdd;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",val);
  TraceValue("%d ",slice_properties[si].size);
  TraceValue("%d ",offset);
  TraceValue("%08x ",mask);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

static void set_value_help_even(dhtElement *he,
                                slice_index si,
                                hash_value_type val)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSuccEven;
  unsigned int const bits = val << offset;
  unsigned int const mask = slice_properties[si].u.h.maskNoSuccEven;
  element_t * const e = (element_t *)he;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",val);
  TraceValue("%d ",slice_properties[si].size);
  TraceValue("%d ",offset);
  TraceValue("%08x ",mask);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceText("\n");
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
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d\n",val);
  TraceValue("%d ",slice_properties[si].size);
  TraceValue("%d ",offset);
  TraceValue("%08x ",mask);
  TraceValue("pre:%08x ",e->data);
  TraceValue("%08x\n",bits);
  assert((bits&mask)==bits);
  e->data &= ~mask;
  e->data |= bits;
  TraceValue("post:%08x\n",e->data);
  TraceFunctionExit(__func__);
  TraceText("\n");
}

static hash_value_type get_value_direct_succ(dhtElement const *he,
                                             slice_index si)
{
  unsigned int const offset = slice_properties[si].u.d.offsetSucc;
  unsigned int const mask = slice_properties[si].u.d.maskSucc;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceValue("%08x ",mask);
  TraceValue("%p ",&e->data);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
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
  TraceFunctionParam("%d ",si);
  TraceValue("%08x ",mask);
  TraceValue("%p ",&e->data);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static hash_value_type get_value_help_odd(dhtElement const *he,
                                          slice_index si)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSuccOdd;
  unsigned int const  mask = slice_properties[si].u.h.maskNoSuccOdd;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceValue("%08x ",mask);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result;
}

static hash_value_type get_value_help_even(dhtElement const *he,
                                           slice_index si)
{
  unsigned int const offset = slice_properties[si].u.h.offsetNoSuccEven;
  unsigned int const  mask = slice_properties[si].u.h.maskNoSuccEven;
  element_t const * const e = (element_t const *)he;
  data_type const result = (e->data & mask) >> offset;
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceValue("%08x ",mask);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
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
  TraceFunctionParam("%d ",si);
  TraceValue("%08x ",mask);
  TraceValue("%08x\n",e->data);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
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
  hash_value_type const succ = get_value_direct_succ(he,si);
  hash_value_type const nosucc = get_value_direct_nosucc(he,si);
  hash_value_type const succ_neg = length-succ;
  assert(succ<=length);
  return succ_neg>nosucc ? succ_neg : nosucc;
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
  hash_value_type const odd = get_value_help_odd(he,si);
  hash_value_type const even = get_value_help_even(he,si);
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p ",he);
  TraceFunctionParam("%u\n",si);

  TraceValue("%u ",odd);
  TraceValue("%u\n",even);

  result = even>odd ? even*2 : odd*2+1;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u\n",result);
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
  switch (slices[leaf].u.leaf.end)
  {
    case EHelp:
      return own_value_of_data_help(he,leaf);

    case EDirect:
    case ESelf:
    case EReflex:
    case ESemireflex:
      return own_value_of_data_direct(he,leaf,1);

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
  TraceFunctionParam("%u\n",si);

  switch (slices[si].u.composite.play)
  {
    case PDirect:
      result = own_value_of_data_direct(he,si,slices[si].u.composite.length);
      break;

    case PHelp:
      result = own_value_of_data_help(he,si);
      break;

    case PSeries:
      result = own_value_of_data_series(he,si);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x\n",result);
  return result;
}

/* Determine the contribution of a stipulation subtree to the value of
 * a hash table element node.
 * @param he address of hash table element to determine value of
 * @param offset bit offset for subtree
 * @param si slice index of subtree root slice
 * @return value of contribuation of the subtree to *he's value
 */
static hash_value_type value_of_data_recursive(dhtElement const *he,
                                               size_t offset,
                                               slice_index si)
{
  hash_value_type result = 0;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p ",he);
  TraceFunctionParam("%u ",offset);
  TraceFunctionParam("%u\n",si);

  offset -= slice_properties[si].value_size;
  TraceValue("%u ",slice_properties[si].value_size);
  TraceValue("->%u\n",offset);

  switch (slices[si].type)
  {
    case STLeaf:
    {
      result = own_value_of_data_leaf(he,si) << offset;
      break;
    }

    case STReciprocal:
    case STQuodlibet:
    {
      hash_value_type const own_value = own_value_of_data_composite(he,si);

      slice_index const op1 = slices[si].u.composite.op1;
      slice_index const op2 = slices[si].u.composite.op2;

      hash_value_type const nested_value1 = value_of_data_recursive(he,
                                                                    offset,
                                                                    op1);
      hash_value_type const nested_value2 = value_of_data_recursive(he,
                                                                    offset,
                                                                    op2);

      hash_value_type const nested_value = (nested_value1>nested_value2
                                            ? nested_value1
                                            : nested_value2);

      result = (own_value << offset) + nested_value;
      break;
    }

    case STSequence:
    {
      hash_value_type const own_value = own_value_of_data_composite(he,si);

      slice_index const op1 = slices[si].u.composite.op1;
      hash_value_type const nested_value =
          value_of_data_recursive(he,offset,op1);
      TraceValue("%x ",own_value);
      TraceValue("%x\n",nested_value);

      result = (own_value << offset) + nested_value;
      break;
    }

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x\n",result);
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
  slice_index const first_slice = 0;
  size_t const offset = sizeof(data_type)*CHAR_BIT;
  hash_value_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%p\n",he);

  TraceValue("%08x\n",((element_t *)he)->data);

  result = value_of_data_recursive(he,offset,first_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%08x\n",result);
  return result;
}

static unsigned long totalRemoveCount = 0;

static void compresshash (void)
{
  dhtElement *he;
  hash_value_type min_val;
  hash_value_type x;
  unsigned long RemoveCnt, ToDelete, runCnt;
#if defined(TESTHASH)
  unsigned long initCnt, visitCnt;
#endif
  size_t val_step = 1;

  ++compression_counter;
  
  he= dhtGetFirstElement(pyhash);
  min_val = value_of_data(he);
  while ((he = dhtGetNextElement(pyhash)))
  {
    x = value_of_data(he);
    if (x<min_val)
      min_val = x;
    he= dhtGetNextElement(pyhash);
  }
  RemoveCnt= 0;
  ToDelete= dhtKeyCount(pyhash)/16 + 1;
  if (ToDelete >= dhtKeyCount(pyhash))
    ToDelete= dhtKeyCount(pyhash);
  /* this is a pathological case: it may only occur, when we are so
   * low on memory, that only one or no position can be stored.
   */

  while ((val_step&min_val)==0)
    val_step <<= 1;

#if defined(TESTHASH)
  printf("\nmin_val: %08x\n", min_val);
  printf("\nval_step: %08x\n", val_step);
  printf("ToDelete: %ld\n", ToDelete);
  fflush(stdout);
  initCnt= dhtKeyCount(pyhash);
#endif  /* TESTHASH */

  runCnt= 0;
  while (RemoveCnt < ToDelete)
  {
    min_val += val_step;

#if defined(TESTHASH)
    printf("min_val: %08x\n", min_val);
    printf("RemoveCnt: %ld\n", RemoveCnt);
    fflush(stdout);
    visitCnt= 0;
#endif  /* TESTHASH */

    for (he = dhtGetFirstElement(pyhash);
         he!=0;
         he= dhtGetNextElement(pyhash))
      if (value_of_data(he)<=min_val)
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

static int TellCommonEncodePosLeng(int len, int nbr_p) {
  int i;

  len++; /* Castling_Flag */
  if (CondFlag[haanerchess]) {
    /*
    ** I assume an average of (nr_files_on_board*nr_rows_on_board -
    ** number of pieces)/2 additional holes per position.
    */
    /* That's far too much. In a ser-h#5 there won't be more
    ** than 5 holes in hashed positions.      TLi
    */
    int nbr_holes= (slices[0].u.composite.play==PSeries
                    ? slices[0].u.composite.length
                    : 2*slices[0].u.composite.length);
    if (nbr_holes > (nr_files_on_board*nr_rows_on_board-nbr_p)/2)
      nbr_holes= (nr_files_on_board*nr_rows_on_board-nbr_p)/2;
    len += bytes_per_piece*nbr_holes;
  }
  if (CondFlag[messigny]) {
    len+= 2;
  }
  if (CondFlag[duellist]) {
    len+= 2;
  }
  if (CondFlag[blfollow] || CondFlag[whfollow] || CondFlag[champursue]) {
    len++;
  }
  if (flag_synchron) {
    len++;
  }
  if (CondFlag[imitators]) {
    for (i = 0; i < inum[nbply]; i++) {
      len++;
    }
    /* coding of no. of imitators and average of one
       imitator-promotion assumed.
    */
    len+=2;
  }
  if (CondFlag[parrain]) {
    /*
    ** only one out of three positions with a capture
    ** assumed.
    */
    len++;
  }
  if (OptFlag[nontrivial]) {
    len++;
  }
  if (slices[0].u.composite.is_exact) {
    len++;
  }
  return len;
} /* TellCommonEncodePosLeng */

static int TellLargeEncodePosLeng(void) {
  square    *bnp;
  int       nbr_p= 0, len= 8;

  for (bnp= boardnum; *bnp; bnp++) {
    if (e[*bnp] != vide) {
      len += bytes_per_piece;
      nbr_p++;  /* count no. of pieces and holes */
    }
  }
  if (CondFlag[BGL])
    len+= sizeof BGL_white + sizeof BGL_black;

  return TellCommonEncodePosLeng(len, nbr_p);
} /* TellLargeEncodePosLeng */

static int TellSmallEncodePosLeng(void) {
  square  *bnp;
  int nbr_p= 0, len= 0;

  for (bnp= boardnum; *bnp; bnp++) {
    /* piece    p;
    ** Flags    pspec;
    */
    if (e[*bnp] != vide) {
      len += 1 + bytes_per_piece;
      nbr_p++;            /* count no. of pieces and holes */
    }
  }
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

  if (CondFlag[imitators]) {
    int i;
    /* The number of imitators has to be coded too to
    ** avoid ambiguities.
    */
    *bp++ = (byte)inum[nbply];
    for (i = 0; i < inum[nbply]; i++) {
      *bp++ = (byte)(isquare[i] - square_a1);
    }
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

  if (slices[0].u.composite.is_exact)
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

  return bp;
} /* CommonEncode */

static void LargeEncode(HashBuffer *hb) {
  byte  *position= hb->cmv.Data;
  byte  *bp= position+nr_rows_on_board;
  int       row, col;
  square a_square= square_a1;

  /* clear the bits for storing the position of pieces */
  memset(position, 0, nr_rows_on_board);

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow) {
    square curr_square = a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right) {
      piece p= e[curr_square];
      if (p!=vide) {
        Flags pspec= spec[curr_square];
        if (!TSTFLAG(pspec, Neutral))
          SETFLAG(pspec, (p < vide ? Black : White));
        p= abs(p);
        if (one_byte_hash)
          *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
        else {
          unsigned int i;
          *bp++ = p;
          for (i = 0; i<bytes_per_spec; i++)
            *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
        }
        position[row] |= BIT(col);
      }
    }
  }

  /* Now the rest of the party */
  bp= CommonEncode(bp);

  hb->cmv.Leng= bp - hb->cmv.Data;
} /* LargeEncode */

static void SmallEncode(HashBuffer *hb)
{
  byte   *bp= hb->cmv.Data;
  int    row, col;
  square a_square= square_a1;

  for (row=0; row<nr_rows_on_board; row++, a_square+= onerow) {
    square curr_square= a_square;
    for (col=0; col<nr_files_on_board; col++, curr_square+= dir_right) {
      piece p= e[curr_square];
      if (p!=vide) {
        Flags pspec= spec[curr_square];
        if (!TSTFLAG(pspec, Neutral))
          SETFLAG(pspec, (p < vide ? Black : White));
        p= abs(p);
        *bp++= (byte)((row<<(CHAR_BIT/2))+col);
        if (one_byte_hash)
          *bp++ = (byte)pspec + ((byte)piece_nbr[p] << (CHAR_BIT/2));
        else {
          unsigned int i;
          *bp++ = p;
          for (i = 0; i<bytes_per_spec; i++)
            *bp++ = (byte)((pspec>>(CHAR_BIT*i)) & ByteMask);
        }
      }
    }
  }

  /* Now the rest of the party */
  bp= CommonEncode(bp);

  hb->cmv.Leng= bp - hb->cmv.Data;
} /* SmallEncode */

boolean inhash(slice_index si,
               hashwhat what,
               hash_value_type val,
               HashBuffer *hb)
{
  boolean result = false;
  dhtElement const * const he= dhtLookupElement(pyhash, (dhtValue)hb);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d ",what);
  TraceFunctionParam("%d\n",val);

  ifHASHRATE(use_all++);

  if (he==dhtNilElement)
    result = false;
  else
    switch (what)
    {
      case SerNoSucc:
      {
        hash_value_type const nosucc = get_value_series(he,si);
        if (slices[si].u.composite.is_exact ? nosucc==val : nosucc>=val)
        {
          ifHASHRATE(use_pos++);
          result = true;
        }
        else
          result = false;
        break;
      }
      case HelpNoSuccOdd:
      {
        hash_value_type const nosucc = get_value_help_odd(he,si);
        if (slices[si].u.composite.is_exact ? nosucc==val: nosucc>=val)
        {
          ifHASHRATE(use_pos++);
          result = true;
        }
        else
          result = false;
        break;
      }
      case HelpNoSuccEven:
      {
        hash_value_type const nosucc = get_value_help_even(he,si);
        if (slices[si].u.composite.is_exact ? nosucc==val : nosucc>=val)
        {
          ifHASHRATE(use_pos++);
          result = true;
        }
        else
          result = false;
        break;
      }
      case DirNoSucc:
      {
        hash_value_type const nosucc = get_value_direct_succ(he,si);
        if (slices[si].u.composite.is_exact ? nosucc==val : nosucc>=val)
        {
          ifHASHRATE(use_pos++);
          result = true;
        } else
          result = false;
        break;
      }
      case DirSucc:
      {
        hash_value_type const succ = get_value_direct_nosucc(he,si);
        if (slices[si].u.composite.is_exact ? succ==val : succ<=val)
        {
          ifHASHRATE(use_pos++);
          result = true;
        } else
          result = false;
        break;
      }

      default:
        assert(0);
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%d\n",result);
  return result; /* avoid compiler warning */
} /* inhash */

/* Initialize the bits representing a direct slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_direct(dhtElement *he,
                                slice_index si,
                                unsigned int length)
{
  set_value_direct_nosucc(he,si,0);
  set_value_direct_succ(he,si,length);
}

/* Initialize the bits representing a help slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_help(dhtElement *he, slice_index si)
{
  set_value_help_even(he,si,0);
  set_value_help_odd(he,si,0);
}

/* Initialize the bits representing a series slice in a hash table
 * element's data field with null values
 * @param he address of hash table element
 * @param si slice index of series slice
 */
static void init_element_series(dhtElement *he, slice_index si)
{
  set_value_series(he,si,0);
}

static void init_element(dhtElement *he, slice_index si);

/* Initialize the bits representing a composite slice (including its
 * descendants) in a hash table element's data field with null values
 * @param he address of hash table element
 * @param si slice index of slice
 * @note this is an indirectly recursive function
 */
static void init_element_composite(dhtElement *he, slice_index si)
{
  switch (slices[si].u.composite.play)
  {
    case PDirect:
      init_element_direct(he,
                          si,
                          slices[si].u.composite.length-slack_length_direct);
      break;
      
    case PHelp:
      init_element_help(he,si);
      break;
      
    case PSeries:
      init_element_series(he,si);
      break;

    default:
      assert(0);
      break;
  }

  switch (slices[si].type)
  {
    case STReciprocal:
    case STQuodlibet:
      init_element(he,slices[si].u.composite.op1);
      init_element(he,slices[si].u.composite.op2);
      break;

    case STSequence:
      init_element(he,slices[si].u.composite.op1);
      break;

    default:
      assert(0);
      break;
  }
}

/* Initialize the bits representing a leaf slice in a hash table
 * element's data field with null values 
 * @param he address of hash table element
 * @param leaf slice index of leaf slice
 */
static void init_element_leaf(dhtElement *he, slice_index leaf)
{
  switch (slices[leaf].u.leaf.end)
  {
    case EHelp:
      init_element_help(he,leaf);
      break;

    case EDirect:
    case ESelf:
    case EReflex:
    case ESemireflex:
      init_element_direct(he,leaf,1);
      break;

    default:
      assert(0);
      break;
  }
}

/* Initialize a hash table element's data field with null values.
 * @param he address of hash table element
 * @param si slice index of root element of stipulation (sub)tree
 * @note this is an indirectly recursive function
 */
static void init_element(dhtElement *he, slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceText("\n");

  if (slices[si].type==STLeaf)
    init_element_leaf(he,si);
  else
    init_element_composite(he,si);

  TraceFunctionExit(__func__);
  TraceText("\n");
}

void addtohash(slice_index si,
               hashwhat what,
               hash_value_type val,
               HashBuffer *hb)
{
  dhtElement *he = dhtLookupElement(pyhash, (dhtValue)hb);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d ",si);
  TraceFunctionParam("%d ",what);
  TraceFunctionParam("%u\n",val);

  if (he == dhtNilElement)
  { /* the position is new */
    he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
    if (he==dhtNilElement
        || dhtKeyCount(pyhash)>MaxPositions)
    {
      compresshash();
      he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
      if (he==dhtNilElement
          || dhtKeyCount(pyhash) > MaxPositions)
      {
#if defined(FXF)
        ifTESTHASH(
            printf("make new hashtable, due to trashing\n"));
        inithash();
        he= dhtEnterElement(pyhash, (dhtValue)hb, 0);
        if (he==dhtNilElement
            || dhtKeyCount(pyhash) > MaxPositions) {
          fprintf(stderr,
                  "Sorry, cannot enter more hashelements "
                  "despite compression\n");
          exit(-2);
        }
#else
        fprintf(stderr,
                "Sorry, cannot enter more hashelements "
                "despite compression\n");
        exit(-2);
#endif /*FXF*/
      }
    }

    he->Data = template_element.Data;

    switch (what)
    {
      case SerNoSucc:
        set_value_series(he,si,val);
        break;

      case HelpNoSuccOdd:
        set_value_help_odd(he,si,val);
        break;

      case HelpNoSuccEven:
        set_value_help_even(he,si,val);
        break;

      case DirSucc:
        set_value_direct_succ(he,si,val);
        break;

      case DirNoSucc:
        set_value_direct_nosucc(he,si,val);
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

      case HelpNoSuccOdd:
        if (get_value_help_odd(he,si)<val)
          set_value_help_odd(he,si,val);
        break;

      case HelpNoSuccEven:
        if (get_value_help_even(he,si)<val)
          set_value_help_even(he,si,val);
        break;

      case DirSucc:
        if (get_value_direct_succ(he,si)>val)
          set_value_direct_succ(he,si,val);
        break;

      case DirNoSucc:
        if (get_value_direct_nosucc(he,si)<val)
          set_value_direct_nosucc(he,si,val);
        break;
    }

  TraceFunctionExit(__func__);
  TraceText("\n");

#if defined(HASHRATE)
  if (dhtKeyCount(pyhash)%1000 == 0)
    HashStats(3, "\n");
#endif /*HASHRATE*/
} /* addtohash */

void inithash(void)
{
  int Small, Large;
  int i, j;

  ifTESTHASH(
      sprintf(GlobalStr, "calling inithash\n");
      StdString(GlobalStr)
      );

#if defined(__unix) && defined(TESTHASH)
  OldBreak= sbrk(0);
#endif /*__unix,TESTHASH*/

#if defined(FXF)
  if (fxfInit(MaxMemory) == -1) /* we didn't get hashmemory ... */
    FtlMsg(NoMemory);
  ifTESTHASH(fxfInfo(stdout));
#endif /*FXF*/

  compression_counter = 0;

  init_slice_properties();
  init_element(&template_element,0);

  PositionCnt= 0;
  dhtRegisterValue(dhtBCMemValue, 0, &dhtBCMemoryProcs);
  dhtRegisterValue(dhtSimpleValue, 0, &dhtSimpleProcs);
  pyhash= dhtCreate(dhtBCMemValue, dhtCopy, dhtSimpleValue, dhtNoCopy);

  ifHASHRATE(use_pos = use_all = 0);

  /* check whether a piece can be coded in a single byte */
  j = 0;
  for (i = PieceCount; Empty < i; i--) {
    if (exist[i])
      piece_nbr[i] = j++;
  }
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
    if (MaxMemory>0 && MaxPositions==0)
      MaxPositions= MaxMemory/(24+sizeof(char *)+1);
  }
  else
  {
    Small= TellSmallEncodePosLeng();
    Large= TellLargeEncodePosLeng();
    if (Small <= Large) {
      encode= SmallEncode;
      if (MaxMemory>0 && MaxPositions==0)
        MaxPositions= MaxMemory/(Small+sizeof(char *)+1);
    }
    else
    {
      encode= LargeEncode;
      if (MaxMemory>0 && MaxPositions==0)
        MaxPositions= MaxMemory/(Large+sizeof(char *)+1);
    }
  }

#if defined(FXF)
  ifTESTHASH(printf("MaxPositions: %7lu\n", MaxPositions));
  ifTESTHASH(printf("MaxMemory:    %7u KB\n", MaxMemory/1024));
#else
  ifTESTHASH(
      printf("room for up to %lu positions in hash table\n", MaxPositions));
#endif /*FXF*/
} /* inithash */

void    closehash(void)
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
    unsigned long HashCount, HashMem, BytePerPos;
#if defined(FXF)
    HashMem= fxfTotal();
#else
    HashMem= sbrk(0)-OldBreak;
#endif /*FXF*/
    if ((HashCount=dhtKeyCount(pyhash))>0) {
      BytePerPos= (HashMem*100)/HashCount;
      sprintf(GlobalStr,
              "Memory for hash-table: %ld, "
              "gives %ld.%02ld bytes per position\n",
              HashMem, BytePerPos/100, BytePerPos%100);
    }
    else {
      sprintf(GlobalStr, "Nothing in hashtable\n");
    }
    StdString(GlobalStr);
#endif /*__unix*/
  }
#endif /*TESTHASH*/

  dhtDestroy(pyhash);

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
