/**************************** MODIFICATIONS to py1.c *****************************
**
** errormsg 46	added.					15.07.1989	NG
**
** errormsg 47	and
** errormsg 48	added.					18.07.1989	NG
**
** function  legalsquare() modified and
** function  nocontact()  added	for koeko-chess.	18.07.1989	NG
**
** errormsg 49	added.					21.07.1989	NG
**
** errormsg 50	added.					22.07.1989	NG
**
** function nocontact()	modified for circe.		22.07.1989	NG
**
** errormsgs  26  and  33  modified.			22.07.1989	NG
**
** gridchess allowed with monochrome or	bichrome chess	22.07.1989	NG
**
** flag	 flappseul for only(!) setplay in helpplay	23.07.1989	NG
**
** randzueger and doppelrandzueger added.		12.10.1989	NG
**
** patrcalcule and patrsoutien hard-initialisation
** changed from	true to	false for better consistency	17.10.1989	NG
**
** due to great	reorganisation function	nocontact()
** transferred to py2.c	and function
** legalsquare() transferred to	py3.c			22.11.1989	NG
**
** some	initialisation transferred from	initvars()
** to hardinit.	a suivre is working now	!		25.11.1989	NG
**
** functions duplexinit() and duplexsquare() added
** for calculation of duplex problems.			26.11.1989	NG
**
** bug fixed in	duplexinit. (Due to this bug
** fairy pieces	were able to jump outside the board !!)	 9.12.1989	NG
**
** errormsgs 51, 52  and new items for PWC added.	17.02.1990	NG
**
** ecritposition modified for "immunschach" and
**		 "ohneschlag".				17.03.1990	NG
**
** errormsgs 51	modified for "immunschach".		17.03.1990	NG
**
** errormsgs  43  and  44  modified.			20.03.1990	NG
**
** function ecritposition() modified for
**	    "immunschach malefique".			20.03.1990	NG
**
** initialisation of e_ubi[] added.			24.03.1990	NG
**
** wrong initializing of vec[] for alfil fixed
** Original popeye bug					 4.03.1990	NG
**
** errormsg and fatal deleted, replaced with seperate	19.04.1990	ElB
** C-Code in pyerrs.c
**
** new option: sansrn   no black king allowed !          3.06.1990      NG
**
** due to new stipulation "Ziel" and problems with my
** Middle Model Compiler old py2.c file inserted here.  14.08.1990      NG
**
** initialisierung von SemiFlag, PattFlag, OptFlag[solmenaces]
**                     und droh ergaenzt                30.8.1990      TLi
**
** due to some speed and code optimizations
** initializationof vec[] deleted.                        13.10.1990      NG
**
** duplex-change of firstpromotionpiece added.          17.10.1990      NG
**
** function nogridcontact() added for contactgridchess.
** function nocontact() modified for contactgridchess.  17.11.1990      NG
**
** function hardinit() modified for imitators           15.01.1991      TM
**
** function hardinit() modified for circe problems
** with kamikaze pieces                                 05.02.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** duplex-change of firstpromotionpiece corrected.      17.02.1991      NG
**
** contactgridchess: rule-bug in nocontact() fixed.     17.03.1991      NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** Initializing of SemiFlag removed			29.07.1991	ElB
**
** neue Funktionen: Store/ResetPosition			17.9.1991	TLi
** alle duplex-Funktionen  geloescht			18.9.1991	TLi
** flaggenre in hardinit  geloescht			20.9.1991	TLi
**
** haanerchess (hole) bug with koeko and circe fixed.   21.12.1991      NG
**
** change(), setneutre(), finligne() jetzt Makros       15.2.1992       TLi
**
** some loop-bugs fixed in hardinit(), Store/ResetPosition()
**                                                      23.02.1992      NG
**
** Sense of OptFlag[iprom] reversed, so with Imitators, promotion to
** an Imitator is always allowed, if not excluded by Option NoIProm.
**                                                      05.04.1992      NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of py1.c to
**                 py1-c.mod. Look there for remarks before this date.
**
** 1993/6/20  TM   Imitator with hopper, fairy-rider and fairy-leaper.
**
** 1993/07/02 StH  The variable solutions is set to 0 in 'hardinit'
**
** 1993/7/3   NG   due to fatal iasm386 bug (of SINIX 5.41) changed
**                 initilisation with  rennormal  inside hardinit().
**
** 1993/07/10 TLi  StorePosition() and ResetPosition() modified.
**
** 1993/07/10 TLi  new option: solflights x - initialization of maxflights
**                 added in hardinit()
**
** 1993/08/05 NG   memset() bugs eliminated.
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/04/29 TLi  new implementation of ForcedSquares
**
** 1994/07/28 TLi  new piece: contragrashopper.
**                 new function for detekting checks of a generalised
**                 rider hopper: riderhoppercheck
**
** 1994/07/29 TLi  initilisation of ep2[..] in hardinit added.
**
** 1994/12/14 TLi  Vogtlaender bug fixed by extending the initilisation
**                 of e[] to e[0] in hardinit().
**
** 1994/12/23 NG   CLEARFL of sq_spec added and initilisation
**                 of sq_spec with SqColor moved inside hardinit();
**
** 1995/01/10 NG   New: e_ubi_mad due to UbiUbi bug with Madrasi.
**
** 1995/04/04 TLi  KoeKo+Circe bug fixed in function nocontact
**
** 1995/04/26 TLi  new stipulation: h#/= etc.
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**
** 1995/06/29 TLi  further attempt to optimize orthodox play
**
** 1995/07/19 TLi  function riderhoppercheck had to be rewritten due to
**                 new piece grasshopper-2
**
** 1995/08/04 NG   Castling in totalortho play implemented.
**
** 1995/08/09 NG   Castling for legalsquare conditions implemented.
**
** 1995/10/13 NG   koeko + sentinelles bug fixed in nocontact().
**            
** 1995/10/31 NG   koeko + circe bug fixed in nocontact().
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
**
** 1995/04/25 TLi  twinning supported
**
** 1996/01/01 NG   Moved all remarks on modifications of py1.c to
**                 py1-c.mod. Look there for remarks before this date.
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12    SE/TLi  new condition: isardam, ohneschach
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/17 NG   Option  PromOnly  changed to Condition  Promonly
**
** 1997/03/10 TLi  generalized Chameleon Circe
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/04 TLi  en passant initilisation bug fixed
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdverse
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutral
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/03/29 TLi  castling implementation changed
**
** 1999/04/02 NG   New option:  NoCastling
**
** 1999/04/17 TLi  new: rex inclusive (for Woozles and Messigny Chess)
**
** 1999/05/25 NG   Moved all remarks on modifications of py1.c to
**                 py1-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/09/27 TLi  new condition: EiffelChess
**
** 1999/10/08 SE   new Sentinelles variants - max white pawns and max black pawns
**
** 2000/01/05 SE   small bug fix in function nocontact for new Sentinelles variants
**
** 2000/01/09 SE   new quasi-condition: maxi (black or white depending on maincamp
**                 new condition: Ultraschachzwang (Bl, W, and quasi versions)
**
** 2000/01/19 SE   new condition : parasentinelles
**
** 2000/01/29 SE   Messigny + Koeko bug fixed
**
** 2000/05/26 NG   koeko + sting: castling bug fixed in nocontact()
**
** 2000/06/05 NG/TLi  bug fix: sentinelles works now with fairy pawns. 
**
** 2001/01/14 SE   new specification : ColourChanging
**
** 2001/02/05 TLi  new piece: Friend
**
** 2001/05/08 SE   new conditions: SentBerolina
**
** 2001/05/20 SE   new stipulation: atob
**
** 2001/10/02 TLi  marin pieces/locust bug fixed in marincheck
**
** 2002/05/18 NG   bug fix: flagAssassin and marsrenai initialized
**                          DiaCirce 'flags and specs' reset
**
** 2004/02/06 SE   Koeko Neighbourhood (invented S.Emmerson)
**
** 2004/04/23 SE   Oscillating Ks TypeC, also allowed A/B/C different for white/black
**
** 2005/04/12 ThM  Bugfix Imitator-Promotions
**
**************************** INFORMATION END ***************************/
