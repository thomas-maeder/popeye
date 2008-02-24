/******************** MODIFICATIONS to pymac.h **************************
**
** Date       Who  What
** 
** 1991       SB   Original
** 
**************************** End of List ******************************/ 

/*
 * This include file is only valid
 * on Macintosh's MPW development environment
 * (otherwise you need it perhaps also, but it's ignored)
 * (It can not be written into the source
 * due to precompiler problems on my PC-X.  NG)
 */
#if defined(SEGM1)
#       pragma segment seg1
#endif

#if defined(SEGM2)
#       pragma segment seg2
#endif

#if defined(SEGMIO)
#       pragma segment segio
#endif
