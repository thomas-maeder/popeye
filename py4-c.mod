/**************************** MODIFICATIONS to py4.c *****************************
**
** Four new pieces: amazon, empress, princess and  gnu  23. 7.1989      NG
**
** More	rose bugs fixed	in testempile()			 7. 8.1989	NG
**
** source better readable: defines used	!
** a bucephale bug fixed in gfeerb()			12.10.1989	NG
**
** a  neutral equihopper  bug fixed.
** (neutral equihopper where able to do	waiting
**  moves ending at the	starting square)		16.10.1989	NG
**
** a great patrol-chess	bug fixed.
** (it just didn't work correct !). Therefore
** functions empile() and genblanc() modified.          17.10.1989      NG
**
** Five	new pieces: antelope, dragon, squirrel,
**		    kangaroo  and  waran		20.11.1989	NG
**
** New piece:	"Spiralspringer"			 4.12.1989	NG
**
** Pawn	and Berolina-Pawn modified for PWC.		17.02.1990	NG
**
** function empile() modified for "immunschach"
**			      and "ohneschlag"		17.03.1990	NG
**
** New piece:	"Ubi-Ubi"				24.03.1990	NG
**
** Due to further enhancements with other shapes of
** the board like cylinder-board, moves of pieces
** to there starting square aren't generated in
** function empile() !! Therefore generation of moves
** in grose[bn]() and gequi[bn]() were modified.        30.05.1990      NG
**
** modification bug fixed in groseb().                  30.06.1990      NG
**
** Four new pieces: hamster, eagle, moose and sparrow    4.07.1990      NG
**
** Three new pieces: archbishop, reflecting bishop and
**                   cardinal                            8.07.1990      NG
**
** Two new pieces: nightrider-hopper, dummy-piece       16.07.1990      NG
**
** Three new pieces: gnu-, camel-, zebra-rider          17.07.1990      NG
**
** Four new pieces: gnu-, camel-,zebra-rider-hopper
**              and DiagonalSpiralspringer              20.07.1990      NG
**
** functions for new pieces and gcs(bn)() modified.     20.07.1990      NG
**
** New piece: bouncy knight                             23.07.1990      NG
**
** gmhop?-bug fixed: testempile() instead of empile()    7.08.1990      NG
**
** function encore() eliminated. It's now a MAKRO !
** goto's in empile() eliminated.                       16.10.1990      NG
**
** functions empile(), testempile(), ge?vec(), modified for
** use for problems with imitators. (test)empile() return
** false if and only if a move cannot be done because of
** imitators.                                           16.01.1991      TM
**
** modification in genblanc(): in circe rex incl., it
** must be avoided that a kamikaze king vanishes while
** taking.                                              04.02.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** flag empilegenre instead of flaggenre in empile()
** and genrb() old stuff deleted.                       11.06.91        NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** finkiller modifiziert
** empile und genblanc modifiziert (kamikaze)
** neu: genweiss,  umgekraempelt: genblanc		18.9.1991	TLi
**
** length() modified: return of precompiled constants,
** mao move-length corrected.                           23.11.91        NG
**
** neu: ge?lst??? fuer schnelleres Generieren in der vorletzten Zugebene
** bei ortho-#-Aufgaben					20.2.92		TLi
**
** equihopper-bug fixed, nonstop-equihopper optimized.  15.03.91        NG
**
** testdebut for optimizing testempile(). (hint of TLi) 06.04.92        NG
**
** All hunter-bugs fixed.                               22.01.1993	NG
**
** genpb modified for Einstein-Chess		        9. 4. 1993	TLi
**
** New pieces: ErlKing, BoyScout, GirlScout              1.05.1993      NG
**
** 1993/5/25  ElB  Moved all remarks on modifications of py4.c to
**                 py4-c.mod. Look there for remarks before this date.
**
** 1993/6/20  TM   hopper with imitator.
**
** 1993/07/10 TLi  new pieces: skylla and charybdis;
**                 new functions geskylchar(), geskylla(), gecharybdis();
**                 gfeer..() modified
**
** 1993/07/10 TLi  number of generating functions for Mao/Moa(riders) and
**                 Hamster reduced (1 function for black and white man of
**                 the same kind) - about 1K smaller code.
**
** 1993/07/31 TLi  number of generating functions for a lot of pieces
**                 reduced
**
** 1993/08/11 NG   number of generating functions for orphans reduced.
**
** 1994/02/15 NG   New conditions:  ForcedSquares
**
** 1994/04/27 TLi  Orphan bug fixed in genpb()
**
** 1994/04/29 TLi  new implementation of ForcedSquares
**
** 1994/07/29 TLi  new: header file py4.h to declare the exported functions
**                 and to have an index
**
** 1994/12/14 TLi  new pieces: RoseLion and RoseHopper. New function for
**                 generating moves groselh. gfeerrest adapted.
**
** 1995/01/11 NG   contragrashopperbug fixed in gfeerrest(): 
**                 a  return;  was missing
**
** 1995/05/26 TLi  new condition: BeamtenChess
**
** 1995/06/01 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/06/14 TLi  new piece: Okapi = Knight + Zebra
**
** 1995/06/29 TLi  further attempt to optimize orthodox play
**
** 1995/07/19 TLi  new piece: 3:7-leaper
**
** 1995/07/19 TLi  function geriderhopper had to be rewritten due to
**                 new piece grasshopper-2
**
** 1995/08/03 TLi  new piece: grasshopper-3
**
** 1995/08/04  NG  Castling in totalortho play implemented.
**
** 1995/08/08 TLi  len_max()/len_min() adapted for castling
**
** 1995/08/09  NG  Castling for a lot of conditions implemented.
**
** 1995/08/18 TLi  new conditions: direct reflex and wh/bl must check
**                 new functions len_direct and len_check
**
** 1995/10/31 NG   enpassant-key bug fixed in gen_p_captures().
**
** 1995/11/02 NG   new piece: 1:6-leaper
**
** 1995/11/04 NG   Some old (#ifdef NODEF) stuff deleted.
**
** 1995/12/08 NG   nK-bug fixed in empile()
**
** 1995/12/28 TLi  old orthodox optimization stuff and switch OPTIMIZE
**                 deleted
**
** 1995/12/28 TLi  didn't succeed in implementing SchachZwang and
**                 directreflex -- deleted both
**
** 1996/01/01 NG   Moved all remarks on modifications of py4.c to
**                 py4-c.mod. Look there for remarks before this date.
**
** 1996/09/17 TLi  rosehopper bug fixed.
**
** 1996/11/06 TLi  new pieces: 2:4-leaper, 3:5-leaper
**
** 1996/12    SE   new pieces: double grashopper and kinghopper 
**
** 1996/12    SE/TLi new condition: Isardam
**
** 1996/12    TLi  new condition: checkless chess
**                 maximummer implementation revised
**
** 1997/02/06 NG   new piece: orix
**
** 1997/02/07 TLi  new restart mechanism
**
** 1997/02/27 TLi  ep-bug in Madrasi fixed
**
** 1997/04/23 NG   illegal array index bug fixed.
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
** 1997/11/10 SE   Mars Circe / Neutral pieces bug fixed
**
** 1997/11/15 SE   Mars Circe bug in cntoppmoves fixed
**
** 1997/12/30 SE   new condition : Republican Chess
**
** 1998/06/10 NG   better usage of  castling_supported
**
** 1998/07/14 TLi  maximummer + neutral pieces bug fixed in empile,
**                 some old stuff removed.
**
** 1998/07/26 TLi  new piece specification: beamtet
**                 function empile() modified
**
** 1998/07/27 TLi  new pieces: rook/bishop/night locust
**
** 1998/10/26 SE   royal pieces now allowed with some circe RI, immune RI
**
** 1999/02/19 NG   new pieces: wazir-/fers-rider (think at circe rebirth !)
**
** 1999/03/15 TLi  new condition: MessignyChess (RexInclusive)
**
** 1999/03/29 TLi  castling implementation changed to fix TransKing bug
**
** 1999/04/17 TLi  new condition: Woozles
**
** 1999/04/20 TLi  maxi/minimummer bug fixed in GenMatingMove
**
** 1999/05/04 TLi  vogtlaender bug fixed in empile
**
** 1999/05/04 TLi  orphan+transking bug fixed in genrb
**
** 1999/05/25 NG   Moved all remarks on modifications of py4.c to
**                 py4-c.mod. Look there for remarks before 1998/01/01.
**
** 1999/09/27 TLi  new condition: EiffelChess
**
** 1999/11/21 TLi  new piece: Bison (1,3)+(2,3) leaper
**
** 2000/01/13 TLi  new piece: Elephant
**
** 2000/04/12 NG   new pieces: (rook/bishop)-(eagle/moose/sparrow)
**                 bug fix for marscirce/phantomchess with fairy-kings (hint of SE)
**
** 2000/05/09 NG   new piece: Rao (Chinese Rose)
**
** 2000/05/09 TLi  optimization of black moves in MarsCirce disabled
**                 yielded mistakes
**
** 2000/05/26 NG   new piece: scorpion  new condition: StingChess
**
** 2000/10/21 NG   implementation of condition No?????Prom changed
**
** 2000/10/23 NG   new conditions: NoWhiteCapture, NoBlackCapture
**
** 2000/10/26 TLi  new piece: Marguerite (MG)
**
** 2000/11/29 TLi  new pieces: 3:6 leaper, night-rider lion, mao-rider
**                 lion
**
**************************** INFORMATION END ***************************/
