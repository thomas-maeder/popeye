*********************** P O P E Y E **********************

Popeye is a program to solve a lot of fairy-chess-problems.
If you enter at least a (fairy-) chess-position and a stipulation
Popeye tries to solve this (fairy-) chess position.
Until now you can enter your input with one out of the
following languages:
english, french, german
For german examples look at the directory BEISPIEL.
Whole language dependent syntax and all the possibilities
of Popeye can be found in the language dependent *.txt files.
For any additional information have a look at the file history.txt .

HAVE FUN !


Contents of this distribution:
	readme.txt:     This file. Contains hints for porting and compiling.
	lizenz.txt:     Popeye License.
	history.txt:    Info about POPEYE-development during his life-time.
	py-????.txt     Documentation written in ????-language.
	makefile.unx    Makefile for unix compilation.
	coderule.txt    Some hints on coding style
	py.h:           Contains constant and type definitions.
	pydata.h:       Contains all extern declared data.
	pymsg.h:        Contains constant definitions for POPEYE I/O.
	pyproc.h:       Contains ANSI-C or K&R-C declarations of all procedures.
	py[1-6].c:      Contain the C-Source of all procedures.
	pyhash.c:       Contains the C-Source of POPEYE dynamic hashing.
	pyio.c:         Contains the C-Source of POPEYE I/O.
	pytime.c:       Contains the C-Source of POPEYE time-measurement.
	pysignal.c:     Contains the C-Source of POPEYE signal handling.
        pydata.c:       Contains only includes and a define to generate the
			the object file for all global data.
	pylang.c:	Contains language dependent procedures.
	pymsg.c:        Contains the Message-file related I/O-procedures.
	py-????.msg:    Message-file for POPEYE I/O in ????-language.
	pie-????.txt:   File for POPEYE LaTeX output in ????-language.
	genpystr.c:     C-Source to convert py-????.msg to py-????.str .
			POPEYE needs py-????.str for language-dependent I/O.
	BEISPIEL:	Directory containing german testfiles 
	EXAMPLES:	Directory containing english testfiles 
	DHT:		Directory containing the dynamic hashing support C-Source files.
	LATEX:		Directory containing files for LaTeX support.

Motto:
There is no C-Code portable to every compiler and every machine.

Nevertheless we want to provide C-Code as much portable as
possible. But there are some problems which can be avoided
if you read these few lines, before compiling POPEYE by yourself.
If you do not want to read any further, take a glance at the
makefile.unx .

A lot of compiling this sources was done with the GNU C-Compiler
Version 1.39 in ANSI-mode.

On all these machines this sources compile without any problems.

1. Enumerations:
	Your compiler may know enumeration types, but handles them
	little strange. Enumerations are only used in py.h and only
	there changes have to be applied. For your convenience
	we provide two awk scripts:
		todef.awk and toenum.awk
	The first translates py.h - which uses enumeration types -
	to a version with #defines instead of enumerations.
	The second does the reverse job, and produces from py.h with
	#defines the enumeration form. If you change/add something
	at this locations, be sure to obeye the statet rules there,
	so that later it is always possible to switch from one
	version to the other.

2. DOS and Borland's Turbo-C
	There should be no problems to compile with Turbo-C.
	Be sure to define DOS and TURBOC.
	You cannot use the makefile written for unix.

3. Compiling for UNIX:
	Copy or link the file makefile.unx to makefile.
	Be sure to define UNIX.
	If your machine is BSD'ish, define also BSD.
	This source compiles with gcc-1.39 in ANSI mode.

4. One special candidate for porting-problems is pytime.c.
	It should compile for every UNIX. But there
	may be some problems with the #include's.
	Be sure to include <sys/params.h> only once.
	A special compile switch for TURBO-C (this file is the
	only file which uses this switch [genpystr.c also uses this or "MSC"
	- TLi]) is provided. For other DOS-compilers it is likely you have
	to adapt or completely rewrite this code.

6. On Atari Computers:
	PROBABLY NOT SUPPORTED NOW !
	It should compile with TURBO C without any problems.
	Use the special compile switch ATARI !
	For ATARI and GNU GCC please look at makefile.unx!

7. On Macintosh Computers:
	PROBABLY NOT SUPPORTED NOW !
	It should compile with MPW without any problems.
	The compile switch  macintosh  will always be set by MPW !

	hello out there !
	this is information for mac-user who want to work with POPEYE.
	the only way we know until now to get input to Popeye is:	
		create an input file (word, ...).
		use the copy-function from the edit-menu.
		then leave your editor and start py ( or just switch to py,
			if you	use MultiFinder or System 7 ).
		now use the paste-function from the edit-menu.
		mark your input-data and press the enter-key or
			command - enter ( like executing a tool or
			command in Apple=s MPW ).
		good luck .

	ps.:	This macintosh version is a release from Stefan Brunzen !	
	You can contact Stefan if you have problems or suggestions for 	
	a better implementation.

	The address is: 	
	Stefan Brunzen
	R"ubezahlstr. 20
	81739 M"unchen 
	Germany	

	tel.:  089 / 600796-30
	fax :  089 / 600796-50
	email: stefan@brunzen.com


8. Microsoft C  PWB:
	PROBABLY NOT SUPPORTED NOW !
	There should be no problems to compile with Microsoft C.

9. IBM MVS:
	PROBABLY NOT SUPPORTED NOW !
	Anybody interested in porting and/or running popeye on IBM
	mainframes under MVS or VM/CMS, please use  #define C370  .
	The name "C370" is due to the IBM C/370 compiler available
	for IBM mainframes.
	This MVS version is a release from Harald Denker !	
	If you have any questions about this MVS version feel free
	to contact him.
	email: Harald_Denker@ms.maus.de  (< 16KB)
	       harry@hal.westfalen.de    (no limits)


10. DOS and GCC
	Only for 80386 and higher !
	It should compile without any problems.
	If you've a working installation of GCC (DJGPP) just
	copy or link the file makefile.unx to makefile.
	Adjust the PATH's of your DJGPP installation accordingly.
	Go to the DHT directory and copy or link the file 
	        makefile.unx to makefile.
	Be sure to define UNIX. DO NOT DEFINE DOS !
	To create an executable pydos32.exe, you have to add a line
		where your preferred 32-Bit extender is concatenated
		to py to generate the binary.
	This may last some time on a 80386.

11. OS/2 and GCC
	NOT SUPPORTED ANYMORE !
	It should compile without any problems.
	Use the special compile switches OS2 and GCC

12. BS2000
	NOT SUPPORTED ANYMORE !

13. Hashing and low memory:
	If you are low on memory, you can save some space
	if you edit the dhtvalue.h of the dht package, and
	link only
		dhtsimpl.o and dhtbcmem.o
		dht.o, dhtvalue.o
	Remove the entries for the
	following three types:
		dhtStringValue, dhtCompactMemoryValue,
		dhtMemoryValue
	in these three places:
		dhtValueType
		dhtValueTypeToString
		dhtProcedures
					ElB 3.May 1994

14. DEC VMS
	NOT SUPPORTED ANYMORE !
	Please use the following command procedures as a hint for
	compiling and linking on your VMS system.

-----  PY.COM  ---- simple command procedure for compiling and linking
$ compile == "cc/define=(VMS,MSG_IN_MEM,HASH)"
$ delete py*.obj;*
$ delete py.exe;*
$ write sys$output "Compiling pyhash.c..."
$ compile pyhash
$ write sys$output "Compiling py1.c..."
$ compile py1
$ write sys$output "Compiling py2.c..."
$ compile py2
$ write sys$output "Compiling py3.c..."
$ compile py3
$ write sys$output "Compiling py4.c..."
$ compile py4
$ write sys$output "Compiling py5.c..."
$ compile py5
$ write sys$output "Compiling py6.c..."
$ compile py6
$ write sys$output "Compiling pydata.c..."
$ compile pydata
$ write sys$output "Compiling pyio.c..."
$ compile pyio
$ write sys$output "Compiling pylang.c..."
$ compile pylang
$ write sys$output "Compiling pymsg.c..."
$ compile pymsg
$ write sys$output "Compiling pytime.c..."
$ compile pytime
$ define lnk$library sys$library:vaxcrtl
$ write sys$output "Linking py.exe..."
$ link/exe=py.exe py.opt/option
-----  PY.OPT  -----   optionfile for the linker
py1
py2
py3
py4
py5
py6
pydata
pyio
pylang
pymsg
pytime
pyhash
[.dht]dht
[.dht]dhtvalue
[.dht]dhtcmem
[.dht]dhtsimpl
[.dht]dhtstrin
[.dht]dhtbcmem
[.dht]dhtmem
-----  DHT.COM  -----  command procedure for compiling the DHT-files
$ delete *.obj;*
$ compile == "cc/define=VAXC"
$ compile dht
$ compile dhtvalue
$ compile dhtcmem
$ compile dhtmem
$ compile dhtsimpl
$ compile dhtstrin
$ compile dhtbcmem
---------------------
						TLi, 06.05.1994


15. Windows 32-Bit environment
	Use the special compile switch WIN32 in any case.
	Additionally use the special compile switch WIN98
		if you have more than 256MB of memory
		and use Win95, Win98 or WinME .

