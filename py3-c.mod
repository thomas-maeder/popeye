/**************************** MODIFICATIONS to py3.c *****************************
**
** functions case(bl/n)echec() modified and
** function  echecc()  modified	for koeko-chess.	18. 7.1989	NG
**
** functions casef(bl/no)ech() added and
** functions case(bl/n)echec() modified for fairy
** pieces in koeko-chess, grid-chess,
** monochrome and bichrome chess.			21. 7.1989	NG
**
** alfil:  bug fixed in	patrfbech() and	patrfnech()	22. 7.1989	NG
**
** equihopper and alfil	supported in koeko-chess.	22. 7.1989	NG
**
** Four	new pieces: amazon, empress, princess
**	       and  gnu					23. 7.1989	NG
**
** function  echecc()  modified	for
** randzueger and doppelrandzueger.			12.10.1989	NG
**
** a great patrol-chess	bug fixed.
** (it just didn't work	correct	!).
** therefore functions soutenu() and echecc() modified.	17.10.1989	NG
**
** more	dababba	- bugs fixed.				23.10.1989	NG
**
** Five	new pieces: antelope, dragon, squirrel,
**		    kangaroo  and  waran		20.11.1989	NG
**
** due to great	reorganisation the main	3*2 checking-
** functions [madrech(), patrech(), case??ech()]
** shortened to	two functions rbechec()	and rnechec().
** therefore I had to add a function eval_ortho() for
** dummy evaluation of checking-correctness.
** function legalsquare() transferred
** from py1.c to this module.
** function libre() enhanced for fairy pieces:
** madrasi with	fairy pieces now possible !
** function echecc() modified for new checking
** functions.						22.11.1989	NG
**
** I tried to optimize fee?echec().			24.11.1989	NG
**
** New piece:	"Spiralspringer"			 4.12.1989	NG
**
** Some	dragon bugs fixed. (No pawn action on
** first / eigth  rank !)
** Pawn	and Berolina-Pawn modified for PWC.		17.02.1990	NG
**
** functions r?circechec() modified for	rex inclusiv
**	     fairy-pieces (no royal pieces !!).
** functions r?immunechec() added for rex inclusiv
**	     fairy-pieces (no royal pieces !!).
** function echecc() modified for
**	    immunschach	rex inclusiv.			20.03.1990	NG
**
** New piece:	"Ubi-Ubi"				24.03.1990	NG
**
** new option: sansrn   no black king allowed !          3.06.1990      NG
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
** Four new pieces: gnu-, camel-, zebra-rider-hopper
**              and DiagonalSpiralspringer              20.07.1990      NG
**
** Some functions for the new pieces modified. New function
** rcsech(). It's now possible to use this function inside
** libre() and both (!!) checking functions.            20.07.1990      NG
**
** New piece: bouncy knight                             23.07.1990      NG
**
** due to new stipulation "Ziel" and problems with my
** Middle Model Compiler this file splitted into
** py2.c and py3.c.                                     14.08.1990      NG
**
** some checking-functions modified. So they can be used inside
** libre() and both (!!) checking functions.            16.10.1990      NG
**
** rbechec() and rnechec() modified for problems with
** imitators                                            16.01.1991      TM
**
** made compilable with MPW on macintosh-computers.     14.02.91        SB
**
** terrible bug in rbechec and rnechec for problems with
** imitators fixed                                      08.04.1991      TM
**
** checking function fee?echec bug fixed with two pieces:
** parts of gnuhop? and dcs? where calculated always !  03.06.1991      NG
**
** revised due to speed enhancement with 24 * 24 board     06.91        NG
**
** echecc: echeccgenre statt flaggenre			19.9.1991	TLi
** r?circech und r?immunech modifiziert fuer madrasi	20.9.1991	TLi
** modifiziert fuer universellere Schach-Tests:
** echecc, r?circech, r?immunech			22.9.1991	TLi
**
** All hunter-bugs fixed.				22.01.1993	NG
**
** function echecc modified for Echecs Bicolores	9. 4. 1993	TLi
**
** 1993/5/25  ElB  Moved all remarks on modifications of py3.c to
**                 py3-c.mod. Look there for remarks before this date.
**
** 1993/07/10 TLi  new pieces: skylla and charybdis - new functions
**                 skylla/charybdischeck()
**
** 1994/04/29 TLi  new implementation of ForcedSquares
**
** 1995/01/10 NG   Changed: rubiech() due to UbiUbi bug with Madrasi.
**
** 1995/10/27 TLi  new anticirce implementation
**
** 1996/01/01 NG   Moved all remarks on modifications of py3.c to
**                 py3-c.mod. Look there for remarks before this date.
**
** 1997/03/14 TLi  ep bug fixed in r?echecc
**
** 1997/06/04 SE/TLi  new condition: MarsCirce
**
** 1997/08/11 NG   new condition: PhantomChess
**
** 1997/08/16 NG   conditions  transmuted/reflected kings
**                 additionally splitted into 
**                 white/black transmuted/reflected kings
**
** 1997/10/28 SE   new conditions: CirceAssassin, PlusChess, Brunner
**
** 1997/12/12 SE   new condition: Extinction Chess
**
** 1998/10/26 SE   royal pieces now allowed with some circe RI, immune RI
**
** 1999/05/25 NG   Moved all remarks on modifications of py3.c to
**                 py3-c.mod. Look there for remarks before 1998/01/01.
**
** 2000/01/26 TLi  Anticirce bug fixed
**
** 2000/04/12 NG   bug fix for marscirce/phantomchess with fairy-kings (hint of SE)
**
** 2001/09/29 ThM  new conditions singleboxtype[123]
**
** 2001/10/26 NG   assertions only with -DASSERT due to performance penalty
**
**************************** INFORMATION END ***************************/
