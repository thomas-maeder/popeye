/**************************** MODIFICATIONS to py2.c *****************************
**
** Four	new pieces: amazon, empress, princess
**	       and  gnu					23. 7.1989	NG
**
** Two rose bugs fixed in feeblech() and
**			  feenoech().			 7. 8.1989	NG
**
** Two dababba bugs fixed in feeblech()	and
**			      feenoech().		11.10.1989	NG
**
** due to great	reorganisation functions
** feeblech() and feenoech() deleted and
** nocontact() transfered from py1.c into this module.	22.11.1989	NG
**
** nocontact() modified (de-bug'd) for PWC.              4.06.1990      NG
**
** due to new stipulation "Ziel" and problems with my
** Middle Model Compiler this file moved to py1.c and
** old file py3.c splitted into py2.c and py3.c         14.08.1990      NG
**
** some ???notlibre() functions deleted due to new usage
** of the normal checking functions inside libre():     16.10.1990      NG
**
** function imok () added for checking if a move is
** legal when there are imitators on the chessboard     15.01.1991      TM
**
** function ridimok () added for checking legality
** of rider moves                                       15.01.1991      TM
**
** function joueim() added, which moves the imitators   16.01.1991      TM
**
** function libre() was too big for TopSpeed C. Split
** up into libre() and feerlibre().                     22.01.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** libre() und feerlibre() modifiziert fuer Kombination
**      mit legalsquare-Bedingungen                     22.9.1991       TLi
** soutenu() modifiziert, neu: eval_madrasi() fuer universellere
**				Schach-Tests		22.9.1991	TLi
**
** neue functions: pion-, cav-, roi-, dame-, fou-, tourcheck
** libre() umgeschrieben, ca. 1,5K weniger Code		18.2.1992	TLi
**
** imitator and (non-stop-) equihopper bugs fixed.      15.03.1992      NG
**
** TransmutingKings bug fixed in pioncheck(), pbcheck(), 
** bspawncheck(), spawncheck(), dragoncheck() !
** CirceParrain bug fixed in pbcheck(), dragoncheck() !	25.02.1993	NG
**
** legalsquare modified for Jigger-Pieces and NewKoeko
**							9. 4. 1993	TLi
** new functions:  maooaridercheck, maoridercheck, moaridercheck,
** 		   b_hopcheck, r_hopcheck
**							10. 4. 1993	TLi
**
** New pieces: ErlKing, BoyScout, GirlScout              1.05.1993      NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of py2.c to
**                 py2-c.mod. Look there for remarks before this date.
**
** 1993/6/20  TM   New function hopimok(): hopper and imitator
**
** 1994/04/27 TLi  EinsteinChess and Orphan bug fixed in pioncheck
**
** 1994/07/28 TLi  new function: contragrascheck
**
** 1994/07/29 TLi  ep capture for Madrasi, Einstein and Berolin Pawn
**                 updated. New function ep_not_libre.
**
** 1994/12/14 TLi  new pieces: RoseLion and RoseHopper - therefore new
**                 check testing functions rlh_check, roselioncheck and
**                 rosehoppercheck
**
** 1995/01/10 NG   Changed: ubicheck() due to UbiUbi bug with Madrasi.
**
** 1995/05/26 TLi  new condition: Beamten Chess
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/14 TLi  PatrolChess/Beamten and Madrasi/paraly. pieces allowed now
**
** 1995/06/14 TLi  new piece: Okapi = Knight + Zebra
**
** 1995/07/19 TLi  new pieces: 3:7-leaper, grasshopper-2
**
** 1995/08/03 TLi  new piece: grasshopper-3
**
** 1995/11/02 TLi  new pieces: 1:6-leaper
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  rosehopper bug fixed
**
** 1996/01/01 NG   Moved all remarks on modifications of py2.c to
**                 py2-c.mod. Look there for remarks before this date.
**
** 1996/11/06 TLi  new piece: 2:4-leaper, 3:5-leaper
**
** 1996/12    SE   new pieces: double grashopper and kinghopper 
**
** 1996/12    SE/TLi  new condition: isardam
**
** 1997/02/06 NG   new piece: orix
**
** 1997/02/27 TLi  ep-bug in Madrasi fixed
**
** 1997/03/14 TLi  ep-bug fixed in pbcheck
**
** 1997/04/22 TLi  exclusive chess
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/06/27 NG   new pieces: 1:5-leaper, 2:5-leaper
**                             gral (alfil + rook-hopper)
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/11/17 SE   Madrasi (Isardam) + Neutrals
**
** 1997/12/12 SE   new condition: Echecs Central
**
** 1997/12/29 NG   bug fix in function libre()
**
** 1997/07/26 TLi  new piece specification: Beamtet
**
** 1997/07/27 TLi  new pieces: rook/bishop/night locust
**
** 1998/10/19 NG   bug fix: nonstop equihopper and patrouille
**
** 1999/03/19 TLi  bug fix: exclusiv chess
**
** 1999/04/17 TLi  new condition: Woozles
**
** 1999/05/25 NG   Moved all remarks on modifications of py2.c to
**                 py2-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/09/27 TLi  new condition: EiffelChess
**
** 1999/11/21 TLi  new piece: bison (1,3)+(2,3) leaper
**
** 2000/01/09 SE   new condition: ShieldedKings
**
** 2000/01/13 TLi  new piece: elephant
**
** 2000/01/28 TLi  new piece: Nao (Chinese Nightrider)
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**                 bug fix for marscirce/phantomchess with fairy-kings (hint of SE)
**
** 2000/05/26 NG   new piece: scorpion  new condition: StingChess
**
** 2000/10/26 TLi  new piece: Marguerite (MG)
**
** 2000/11/29 TLi  new pieces: 3:6 leaper, night-rider lion, mao-rider
**                 lion
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Moariderlion
**
** 2001/10/02 TLi  new piece: Dolphin (Grashopper + Kangaroo)
**
** 2002/05/18 NG   new pieces: rabbit, bob
**
**************************** INFORMATION END ***************************/
