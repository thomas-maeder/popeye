*********************** MODIFICATIONS to py.h **************************
**
** boolkamikazezz and boolneutrezz changed to
** bool_kamikaze  and bool_neutre
** because my C-Compiler
** looks only for the first 7 chars per identifier !
**                                                     13. 7.1989      NG
**
** Four new pieces: amazon, empress, princess
** and  gnu                                            23. 7.1989      NG
**
** maxsol (the maximum number of solutions allowed)
** changed from 80 to 100, because I think I've got problems
** with two-movers with 30 and more pieces on board.
**                                                      3. 8.1989      NG
**
** toppile changed from 400 to 500 because I've got
** problems with a 6# where nbcou raised to 407,
** which caused a core-dump.
** also maxply changed from 15 to 19 because
** POPEYE needs for a 9# space of 19 halfmoves.         
**                                                      1.10.1989      NG
**
** an alfil and fers bug fixed: black defines
** were in wrong order !!                       
**                                                     12.10.1989      NG
**
** new definition of boolnoedge for  Randzueger  and
** Doppel-Randzueger !                         
**                                                     12.10.1989      NG
**
** some modifications to get smaller code:
** integer = short ...                         
**                                                     18.10.1989      NG
**
** Five new pieces: antelope, dragon, squirrel,
**                  kangaroo  and  waran               20.11.1989      NG
**
** New piece:   "Spiralspringer"                        4.12.1989      NG
**
** toppile changed from 500 to (50 * maxply)  because I've got
** problems with a s#8 where nbcou should have been about 800.
** 500 caused sudden stop without much calculation.
**                                                      27.02.1990      NG
**
** New piece:   "Ubi-Ubi"                               24.03.1990      NG
**
** new option: sansrn   no black king allowed !          3.06.1990      NG
**
** maxply changed from 19 to 29 because due to speedy hardware (and
** optimizations of POPEYE in the near future !) it will be
** possible to calculate "Wenigsteiner" greater than #9.
**                                                       1.10.1989      NG
**
** Four new pieces: hamster, eagle, moose and sparrow    4.07.1990      NG
**
** Three new pieces: archbishop, reflecting bishop and
**                   cardinal                            8.07.1990      NG
**
** Two new pieces: nightrider-hopper, dummy-piece       16.07.1990      NG
**
** Three new pieces: gnu-rider, camel-rider, zebra-rider
**                                                      17.07.1990      NG
**
** Four new pieces: gnu-rider-hopper, camel-rider-hopper,
**                  zebra-rider-hopper and
**                  DiagonalSpiralSpringer              20.07.1990      NG
**
** New piece: bouncy knight                             23.07.1990      NG
**
** Type boolean changed from char to int due to
** problems with patrolchess (with grashoppers) on
** NSC-machines (NS 32532).                             31.07.1990      NG
**
** optionen solmenaces hinzugefuegt                     13.8.1990   TLi
** semi-reflex- und patt-forderungen ergaenzt           27.8.1990   TLi
**
** SemiRefMate and SemiRefStaleMate shortened because the first
** 7 characters must (!) be different.(see also first comment above !).
**                                                       9.9.1990       NG
**
** datatypes boolean and piece changed to char with
** new compile-option SIXTEEN for 16-bit machines.      15.10.1990  NG
**
** function encore() defined as MAKRO.                  16.10.90        NG
**
** new condition: contactgrid.                          17.11.90        NG
**
** new conditions: imitators and iprom                  17.01.91        TM
** added maxinum, imarr                                 17.01.91        TM
** changed type coup for imitators                      17.01.91        TM
** changed type coup for circe problems with kamikaze
** pieces                                               05.02.91        TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** maxsol (the maximum number of solutions allowed) is now 50
** on SIXTEEN Bit machines to solve more-movers with more than 5 moves.
**                                                      17.02.1991      NG
**
** added macros (rid)imcheck and (rid)imech for
** easier check testing with imitators                  09.04.91        TM
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** A lot of new Macros for Setting and Testing the
** new Stipulation variable                             29.07.91        ElB
**              -- die meisten wieder geloescht !       18.9.1991       TLi
** Macro DBG added, for better readable debug code      29.07.91        ElB
**
** en_eche? nicht mehr benoetigt - geloescht            18.9.91         TLi
** tabsol modifiziert                                   18.9.91         TLi
** CondFlag: patrcalcule und patrsoutien geloescht
**      - CondFlag[patrcalcule] und CondFlag[patrouille] waren stets gleich
**      - patrsoutien jetzt einfacher Flag (pydata.h)   20.9.91         TLi
** neue Forderungungen: coucou, equipollents, chamcirce, filecirce      TLi
**
** maxply changed from 29 to 41 under UNIX because POPEYE is really quick now.
** constant "1000" replaced by toppile, maxsol deleted.  4.10.1991      NG
**
** Conditions "randzueger" and "doppelrandzueger" changed to
** blackedge and whiteedge (!).                         28.11.1991
**
** New cpp constant: VERSION should be set in py.h or Makefile.
**                                                      12Apr1992       ElB
**
** neue Bedingungen: bl/whfollow, duellanten            23.4.1992  TLi
**
** Coucou changed to new condition Couscous.             6.05.1992      NG
**
** New pieces: ErlKing, BoyScout, GirlScout              1.05.1993      NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of py.h to
**                 py-h.mod. Look there for remarks before this date.
**
** 1993/6/20  TM   new function hopimcheck() for hoppers with imitator.
**
** 1993/07/02 StH  I added the option maxsols, which is used to
**                 stop the solving, when the given number of
**                 solutions was encountered.
**
** 1993/07/10 TLi  I modified the storing and reseting of the initial
**                 position to save memory; therefor delete definition of
**                 sic_piece and sic_spec.
**
** 1993/07/10 TLi  new option: solflights x
**
** 1993/07/10 TLi  new pieces: Skylla and Charybdis
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/02/25 NG   Maxply increased: UNIX: 502, others: 102
**
** 1994/04/29 TLi  new implementation of ForcedSquares and MagicSquares
**                 new structure sq_spec = square specifications
**
** 1994/04/29 NG   toppile decreased due to hint from TLi.
**                 Some old (#ifdef NODEF) stuff deleted.
**
** 1994/06/05 NG   Maxply increased: UNIX: 702, others: 102
**
** 1994/07/25 TLi  toppile reset to its old value since NG has misunderstood
**                 me: tab.liste[..] should be decreased
**
** 1994/07/28 TLi  new piece: contra grashopper
**
** 1994/08/10 TLi  declaration of exist[] had to be changed due to
**                 problems with BorlandC3.1
**
** 1994/08/17 TLi  two new options for my database project: search, multi
**                 and nosymmetry
** 1994/12/14 TLi  new pieces: RoseLion and RoseHopper
**
** 1994/04/26 TLi  seriesmovers with introductory move
**
** 1995/05/09 NG   TESTRATE definition moved into this include-file.
**
** 1995/05/26 TLi  ChameleonChess and Beamtenchess
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/08 TLi  new option: nontrivial n m for direct/self problems
**                 White must play moves leaving black with n or less moves
**                 not leading to #m and s#m, resp.
**
** 1995/06/12 TLi  new option: keepmatingpiece - in s,r# the last black piece
**                 must not be captured
**
** 1995/06/14 TLi  new piece: Okapi = Knight + Zebra
**
** 1995/07/19 TLi  new piece: 3:7-leaper
**
** 1995/07/19 TLi  new piece: Grasshopper-2
**
** 1995/07/19 TLi  new stipulation: dia == proof game
**
** 1995/08/03 TLi  new piece: Grasshopper-3
**
** 1995/08/18 TLi  new option: enpassant for ep keys
**
** 1995/08/18 TLi  new conditions: SchachZwang, directreflex
**
** 1995/11/02 NG   new piece: 1:6-leaper
**
** 1995/11/03 NG   new option: noboard - do not print the board to screen or file.
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/28 TLi  new anticirce implementation
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1995/12/29 TLi  new condition: GlasgowChess
**
** 1995/12/30 TLi  new option: noshort == no short variations
**
** 1996/01/01 NG   Moved all remarks on modifications of py.h to
**                 py-h.mod. Look there for remarks before this date.
**
** 1996/02/07 NG   new option: halfduplex, ie. stipulation is only
**                             calculated for the 'opponent' side.
**
** 1996/03/19 TLi  new: twinning supported
**
** 1996/04/23 TLi  new option: postkeyplay
**
** 1996/05/18 TLi  new conditions: AntiAndernachChess, FrischaufCirce
**
** 1996/10/31 NG   new condition: VerticalMirrorCirce
**                 maxply for OS/2 increased to 302
**
** 1996/11/06 TLi  new pieces: 2:4-leaper, 3:5-leaper
**
** 1996/11/15 NG   New option:  PromOnly
**
** 1996/12    SE   new pieces: double grashopper and kinghopper 
**
** 1996/12    SE/TLi  new condition: isardam, ohneschach
**
** 1997/02/06 NG   new piece: orix
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/10 TLi  new condition: DiametralCirce
**
** 1997/02/17 NG   Option  PromOnly  changed to Condition  Promonly
**
** 1997/03/11 TLi  new condition: RankCirce
**
** 1997/03/14 TLi  new option: intelligent
**
** 1997/04/22 TLi  new condition: ExclusiveChess
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/06/27 NG   new pieces: 1:5-leaper, 2:5-leaper
**                             gral (alfil + rook-hopper)
**
** 1997/07/28 TLi  new condition: MarsMirrorCirce
**
** 1997/08/01 TLi  new: LaTeX output, award
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/09/04 TLi  new conditions: AntiEinsteinSchach, CousCourMirrorCirce
**
** 1997/10/02 TLi  new condition: Black/WhiteRoyalSquare
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/10/28 SE   new condition: Isardam TypeB
**
** 1997/11/05 SE   new condition: Sentinelles (en) PionAdvers
**                 AntiCirce variants modified
**
** 1997/11/09 SE   new condition: Sentinelles (en) PionNeutre
**
** 1997/12/12 SE   new conditions: Extinction Chess, Echecs Central
**
** 1997/12/30 SE   new condition : Republican Chess
**
** 1998/06/05 NG   new option: MaxTime
**
** 1998/07/26 TLi  typedef definition of Flags changed to unsigned long
**                 maxply reduced to 45 for DOS
**                 new piece specification: beamtet
**
** 1998/07/27 TLi  new pieces: rook/bishop/night locust
**
** 1998/07/30 NG   maxply reduced to 40 for DOS
**
** 1998/08/21 TLi  new condition: Actuated Revolving Board
**
** 1999/01/10 NG   maxply reduced to 34 for DOS
**
** 1999/02/18 NG   VERSION now OS-binary dependent
**
** 1999/02/19 NG   new pieces: wazir-/fers-rider (think at circe rebirth !)
**                 definitions of queen, rook and knight changed 
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/04/02 NG   New option:  NoCastling
**
** 1999/04/17 TLi  new condition: (Bi)Woozles/Heffalumps
**		   new: rex exclusiv (for Woozles,Heffalumps and Messigny Chess)
**
** 1999/05/02 NG   maxply reduced to 32 for DOS
**
** 1999/05/25 NG   Moved all remarks on modifications of py.h to
**                 py-h.mod. Look there for remarks before 1998/01/01.
**
** 1999/06/12 SE   new piece specification : halfneutrals
**
** 1999/06/22 NG   new conditions: WhitePromSquares  and  BlackPromSquares
**                 new conditions: NoWhitePromotion  and  NoBlackPromotion
**
** 1999/09/23 NG   maxply reduced to 28 for 16 Bit DOS
**                 16 Bit DOS support not longer than 31.12.1999 !
**
** 1999/09/28 TLi  new condition: EiffelChess
**
** 1999/10/07 NG   mao  to  vao  bugfix due to changed orthodox defines.
**
** 1999/10/08 SE   new option : Quodlibet
**                 in self- and (semi-)reflex play, allows White to make contract by
**                 direct stip also (eg. #/S# or #/R#).
**
** 1999/11/08 NG   New option:  StopOnShortSolutions
**
** 1999/11/14 NG   maxply reduced to 26 for DOS (16 Bit)
**
** 1999/11/21 TLi  New piece:  Bison
**
** 2000/01/05 NG   maxply not reduced when compiling 32 Bit DOS with
**                 a real DOS compiler ...
**
** 2000/01/09 SE   new quasi-condition: maxi (black or white depending on maincamp
**                 new condition: Ultraschachzwang (Bl, W, and quasi versions)
**                 new conditions: ActuatedRevolvingCentre, ShieldedKings
**
** 2000/01/13 TLi  new piece: Elephant
**
** 2000/01/26 TLi  Anticirce bug fixed
**
** 2000/01/28 TLi  new piece: Nao (Chinese Nightrider)
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**
** 2000/05/09 NG   new piece: Rao (Chinese Rose)
**
** 2000/05/25 NG   new piece: Scorpion new condition: StingChess
**
** 2000/06/06 TLi  minor adaption for my database project
**
** 2000/09/19 TLi  new condition: LineChameleonChess
**
** 2000/10/20 NG   new compilation flag:  SIXTYFOUR  for 64-Bit CPU's
**
** 2000/10/26 TLi  new piece: Marguerite (MG)
**
** 2000/10/23 NG   new conditions: NoWhiteCapture, NoBlackCapture
**
** 2000/11/14 ThM  new condition: AprilChess
**
** 2000/11/26 TLi,NG VERSIONSTRING more detailed ...
**
** 2000/11/29 TLi  new pieces: 3:6 leaper, night-rider lion, mao-rider lion
**
** 2000/12/14 NG   maxply increased to 1002 for UNIX
**
**************************** End of List ******************************/
