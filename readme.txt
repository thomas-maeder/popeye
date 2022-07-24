*********************** P O P E Y E **********************

Popeye is a program to solve a lot of orthodox and fairy chess problems.
If you enter at least a (fairy-) chess-position and a stipulation
Popeye tries to solve this (fairy-) chess position.
Until now you can enter your input in one out of the
following languages: english, french, german
For german examples look at the directory BEISPIEL.
Whole language dependent syntax and all the possibilities
of Popeye can be found in the language dependent *.txt files.

HAVE FUN !


Contents of this distribution:
	readme.txt:     This file. Contains hints for porting and compiling.
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

A lot of compiling this sources was done with the GNU C-Compiler (gcc) and
cross compilers based on gcc.
