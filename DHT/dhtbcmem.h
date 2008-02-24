#if !defined(DHTBCMEM_INCLUDED)
#define DHTBCMEM_INCLUDED
/* This is dhtbcmem.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is keept
 * intact and in place.
 */


/* This is the Byte Counted Memory Value.
 * It is allocated via one malloc call.
 */

typedef unsigned char uChar;

typedef struct BCMemValue {
	unsigned char	Leng;
	unsigned char	Data[1];
} BCMemValue;
#define NilBCMemValue		(BCMemValue *)0
#define NewBCMemValue(n)	(BCMemValue *)fxfAlloc(sizeof(BCMemValue)-1+(n)*sizeof(uChar))
#define FreeBCMemValue(x)	fxfFree(x, (sizeof(BCMemValue)-1)+(((BCMemValue *)x)->Leng*sizeof(uChar)))

BCMemValue *BCMemValueCreate(int n);
#endif /*DHTBCMEM_INCLUDED*/
