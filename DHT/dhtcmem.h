#if !defined(DHTCMEM_INCLUDED)
#define DHTCMEM_INCLUDED
/* This is dhtcmem.h --  Version 1.5
 * This code is copyright by
 *	Elmar Bartel 1993-99
 *	Institut fuer Informatik, TU Muenchen, Germany  
 *	bartel@informatik.tu-muenchen.de
 * You may use this code as you wish, as long as this
 * comment with the above copyright notice is kept
 * intact and in place.
 */

/* This is the counted bytestring.
 * It is allocated via one malloc call.
 */
typedef struct CompactMemVal {
	unsigned long	Leng;
	unsigned char	Data[1];
} CompactMemVal;
#define NilCompactMemVal	((CompactMemVal *)0)
#define NewCompactMemVal(n)	((CompactMemVal *)fxfAlloc(sizeof(CompactMemVal)+(n)*sizeof(uChar)))
#define FreeCompactMemVal(v)	fxfFree(v, sizeof(CompactMemVal)+((CompactMemVal const *)(v))->Leng*sizeof(uChar))
#endif /*DHTCMEM_INCLUDED*/
