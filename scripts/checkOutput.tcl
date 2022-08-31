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
    namespace eval intro {
	set emptyLine {\n}
	set textLine { +[^\n]+\n}
	set combined "$emptyLine+(?:$textLine)*"
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

	set spaceLine "$verticalBorderSign (?:    ){$nrColumns}  $verticalBorderSign\n"

	set rowNo {[1-8]}
	set squareEmpty {  [.]}
	set color {[ =-]}
	set pieceLetter {[[:upper:]]}
	set piece1Letter " $color$pieceLetter"
	set piece2Letters "$color$pieceLetter{2}"
	set pieceSpec "(?:$squareEmpty|$piece1Letter|$piece2Letters)"
	set piecesLine "$rowNo (?:$pieceSpec ){$nrColumns}  $rowNo\n"

	set stipulation {\#[[:digit:]]+}
	set piecesOfColor {[[:digit:]]+}
	set plus {[+]}
	set pieceControl "$piecesOfColor $plus $piecesOfColor"
	set captionLine "  $stipulation +$pieceControl\n"
	
	set combined "$emptyLine${columns}(?:$spaceLine$piecesLine){$nrRows}$spaceLine$columns$captionLine"
    }

    namespace eval conditions {
	set line { +[^\n]+\n}
	set combined "(?:$line)*"
    }

    namespace eval solution {
	set emptyLine {\n}
	set combined "$emptyLine"
    }

    set bodyRest {.+?}
    
    set endlines {Loesung beendet[.]\n+?}
    
    set problem "($intro::combined$board::combined$conditions::combined$solution::combined)${bodyRest}($endlines)"

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
