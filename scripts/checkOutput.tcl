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
    set introEmptyLine {\n}
    set introTextLine { +[^\n]+\n}
    set intro "$introEmptyLine+(?:$introTextLine)*"

    set boardEmptyLine {\n}
    set boardColumns {[+]---a---b---c---d---e---f---g---h---[+]\n}
    set boardSpaceLine {[|]                                   [|]\n}
    set boardLineNo {[1-8]}
    set boardSquareEmpty {  [.]}
    set boardPiece1Letter { [ =-][A-Z]}
    set boardPiece2Letters {[ =-][A-Z]{2}}
    set boardPieceSpec "(?:(?:$boardSquareEmpty|$boardPiece1Letter|$boardPiece2Letters) )"
    set boardPiecesLine "$boardLineNo $boardPieceSpec{8}  $boardLineNo\n"
    set boardCaptionLine {  \#[0-9]+ +[0-9]+ [+] [0-9]+\n}
    set board "$boardEmptyLine${boardColumns}(?:$boardSpaceLine$boardPiecesLine){8}$boardSpaceLine$boardColumns$boardCaptionLine"

    set conditionLine { +[^\n]+\n}
    set conditions "(?:$conditionLine)*"

    set solutionEmptyLine {\n}
    set solution "$solutionEmptyLine"

    set bodyRest {.+?}
    
    set endlines {Loesung beendet[.]\n+?}
    
    set problem "($intro$board$conditions$solution)${bodyRest}($endlines)"

    set problems "(?:$problem)+?"
}

set f [open $inputfile "r"]
set input [read $f]
close $f

set matches [regexp -all -inline [set ${language}::problems] $input]

foreach { whole top bottom } $matches {
    puts -nonewline $top
    puts -nonewline $bottom
}
