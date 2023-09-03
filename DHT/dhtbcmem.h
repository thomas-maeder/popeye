#if !defined(DHTBCMEM_INCLUDED)
#define DHTBCMEM_INCLUDED
/* This is dhtbcmem.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept
 * intact and in place.
 */


/* This is the Byte Counted Memory Value.
 * It is allocated via one malloc call.
 */

typedef struct BCMemValue {
	unsigned char	Leng;
	unsigned char	Data[1];
} BCMemValue;

#endif /*DHTBCMEM_INCLUDED*/
