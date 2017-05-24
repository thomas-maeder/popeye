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
	*.h, *.c	C source code
	py-????.msg:    Message-file for POPEYE I/O in ????-language.
	pie-????.txt:   File for POPEYE LaTeX output in ????-language.
	BEISPIEL:	Directory containing german testfiles 
	EXAMPLES:	Directory containing english testfiles 
	REGRESSIONTESTS	Output when running the examples in BEISPIEL and EXAMPLES
	DHT:		Directory containing the dynamic hashing support C-Source files.
	LATEX:		Directory containing files for LaTeX support.
	scripts:	Some bash scripts useful for developers
	toolchains:	contain makefile additions for different tool chains

Motto:
There is no C code portable to every compiler and every machine.

Nevertheless we want to provide C code as portable as possible. But
there are some problems which can be avoided if you read these few
lines, before compiling POPEYE by yourself.  If you do not want to
read any further, take a glance at the makefile.unx .

A lot of compiling this sources was done with the GNU C-Compiler
Version 4.xx in ANSI-mode.

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

2. One special candidate for porting-problems is pytime.c.
	It should compile for every UNIX. But there
	may be some problems with the #includes.
	Be sure to include <sys/params.h> only once.
	A special compile switch for TURBO-C (this file is the
	only file which uses this switch [genpystr.c also uses this or "MSC"
	- TLi]) is provided. For other DOS-compilers it is likely you have
	to adapt or completely rewrite this code.

3. Compiling for UNIX, Cygwin, MingW, MacOS X:
	Copy or link the file makefile.unx to makefile.
	The default tool chain used is gcc, with the target platform
	equal to the host platform (i.e. no cross compilation).
	To cross compilation, set the TOOLCHAIN parameter in the make
	command; e.g.:
		make TOOLCHAIN=cross-i386-mingw32msvc
	to cross compile from Linux for MingW (Windows 32). See the
	toolchains directory for support toolchains; and,of course,
	feel free to add other tool chains!

4. Borland C++BuilderX
	Copy or link the file makefile.unx to makefile.
	Make sure that the C++BuilderX (e.g. bcc32.exe) executables
	are in your PATH of a Cygwin bash shell. Do
        	make TOOLCHAIN=C++BuilderX
        In principle, this should also work for C++Builder (without
        the X), but hasn't been tested, feedback is welcome!

5. Microsoft Visual C++
	Copy or link the file makefile.unx to makefile.
	Make sure that the C++BuilderX (e.g. bcc32.exe) executables
	are in your PATH of a Cygwin bash shell. Do
        	make TOOLCHAIN=MSVC6
        As "MSVC6" tells you, this was tested with Visual C++ 6. In
        principle, later Visual C++s (Visual Studios) should work as
        well, but they haven't been tested; feedback is welcome!

6. On older (than MacOS X) Macintosh Computers:
	Popeye should compile with MPW without any problems.
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

7. DOS and GCC
	Only for 80386 and higher !
	Popeye should compile without any problems.
	If you've a working installation of GCC (DJGPP) just
	copy or link the file makefile.unx to makefile.
	Adjust the PATH's of your DJGPP installation accordingly.
	Go to the DHT directory and copy or link the file 
	        makefile.unx to makefile.
	Be sure to define UNIX. DO NOT DEFINE DOS !
	To create an executable pydos32.exe, you have to add a line
		where your preferred 32-Bit extender is concatenated
		to py to generate the binary.
	This may take some time on a 80386.

8. Hashing and low memory:
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

-----  PY.COM  ---- simple command procedure for compiling and linking
$ compile == "cc/define=(VMS,HASH)"
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
