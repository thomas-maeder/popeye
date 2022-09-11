#! /usr/bin/env tclsh

# Usage: $argv0 INPUTFILEPATH | diff -y --width=200 INPUTFILEPATH -

set inputfile [lindex $argv 0]

switch -re $inputfile {
    ".*[.]out$" {
	set language "german"
    }
    ".*[.]tst$" -
    ".*[.]ref$" -
    ".*[.]reg$" {
	set language "english"
    }
    default {
	error "failed to detect language"
	exit 1
    }
}

namespace eval german {
    set endlines {(?:Loesung beendet[.]|Partielle Loesung)\n+?}
    set white {Weiss}
    set black {Schwarz}
}

namespace eval intro {
    set emptyLine {\n}
    set leadingBlanks { *}

    set author {[^\n]+}
    set authorLine "$leadingBlanks$author\n"

    set origin {[^\n]+}
    set originLine "$leadingBlanks$origin\n"

    set award {[^\n]+}
    set awardLine "$leadingBlanks$award\n"

    set title {[^\n]+}
    set titleLine "$leadingBlanks$title\n"

    set combined "$emptyLine+(?:$authorLine)*(?:$originLine)*(?:$awardLine)?(?:$titleLine)?"
}

namespace eval board {
    set nrRows 8
    set nrColumns 8
    
    set emptyLine {\n}

    set cornerSign {[+]}
    set verticalBorderSign {[|]}
    set horizontalBorderSign "-"
    
    set columnName {[a-h]}
    set columnSpec "$horizontalBorderSign$horizontalBorderSign$columnName$horizontalBorderSign"
    set columns "$cornerSign${horizontalBorderSign}(?:$columnSpec){$nrColumns}$horizontalBorderSign$horizontalBorderSign$cornerSign\n"

    set rowNo {[1-8]}
    set gridVertical {[|]}
    set gridHorizontal " ---"
    set noPiece {[.]}
    set squareEmpty "(?: |$gridVertical) $noPiece"
    set hole "(?: |$gridVertical)  "
    set color {[ =-]}
    set pieceChar {[[:upper:][:digit:]]}
    set piece1Char "(?: |$gridVertical)$color$pieceChar"
    set piece2Chars "(?:$color|$gridVertical)$pieceChar{2}"
    set pieceSpecSeparator " "
    set hunterPartsSeparator "/"
    set pieceSpec "(?:$squareEmpty|$hole|$piece1Char|$piece2Chars)(?:$pieceSpecSeparator|$hunterPartsSeparator)"
    set piecesLine "${rowNo} (?:$pieceSpec){$nrColumns}  $rowNo\n"

    set hunter2ndPart "(?:$hole|$piece1Char|$piece2Chars)"
    set spaceLine "$verticalBorderSign (?:$hunter2ndPart$pieceSpecSeparator|$gridHorizontal){$nrColumns}  $verticalBorderSign\n"

    set goal {(?:\#|=|dia|a=>b|z[a-h][1-8]|ct|<>|[+]|==|00|%|~|\#\#|\#\#!|!=|ep|x|ctr|c81|\#=)}
    set exact {(?:exact-)}
    set intro {(?:[[:digit:]]+->)}
    set series "(?:$intro?ser-)"
    set help "h"
    set paren_open {[(]}
    set paren_close {[)]}
    set recigoal "(?:$paren_open$goal$paren_close)"
    set recihelp "(?:reci-h$recigoal?)"
    set self "s"
    set reflex {(?:(?:semi-)?r)}
    set play "(?:(?:$series)?(?:$help|$recihelp|$self|h$self|$reflex|h$reflex)?)"
    set length {(?:[[:digit:]]+(?:[.]5)?)}
    set stipulation_traditional "(?:$exact?$play$goal$length)"

    set side "(?:[set ${language}::white]|[set ${language}::black])"
    set stipulation_structured [subst -nocommands {(?:$side [^ ]+)}]; # TODO

    set maxthreat {(?:/[[:digit:]]*)}
    set maxflight {(?:/[[:digit:]]+)}
    set nontrivial {(?:;[[:digit:]]+,[[:digit:]]+)}

    set stipulation "(?:(?:$stipulation_traditional|$stipulation_structured)(?:$maxthreat$maxflight?)?$nontrivial?)?"; # TODO order of suffixes?
    
    set piecesOfColor {[[:digit:]]+}
    set plus {[+]}
    set pieceControl "$piecesOfColor $plus ${piecesOfColor}(?: $plus ${piecesOfColor}n)?"
    set captionLine " *$stipulation *$pieceControl\n"

#    puts $captionLine; exit 0

    set combined "$emptyLine${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns$captionLine"
}

namespace eval conditions {
    set line { *[^\n]+\n}
    set combined "(?:$line)*"
}

namespace eval solution {
    set emptyLine {\n}
    set combined "$emptyLine"
}

set bodyRest {.+?}

set problem "($intro::combined$board::combined$conditions::combined)${bodyRest}([set ${language}::endlines])"

set problems "(?:$problem)+?"

set f [open $inputfile "r"]
set input [read $f]
close $f

set matches [regexp -all -inline $problems $input]

foreach { whole top bottom } $matches {
    puts -nonewline $top
    puts -nonewline $bottom
}
