#! /usr/bin/env tclsh

# Usage: $argv0 INPUTFILEPATH | diff -y --width=200 INPUTFILEPATH -

switch -re [lindex $argv 0] {
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

set inputfiles $argv

source output/plaintext/documentation/german
source output/plaintext/documentation/english


proc literal {name} {
    global language
    return [set ${language}::$name]
}

# syntactic sugar for looking up variables in ancestor namespaces
proc lookupValue {name} {
    set scope [uplevel namespace current]
    set initialscope $scope
    while {![info exists ${scope}::$name]} {
        if {$scope=="::"} {
            puts stderr "can't find variable $name from scope $initialscope"
            exit 1
        } else {
            set scope [namespace parent $scope]
        }
    }
    return [set ${scope}::$name]
}

proc terminal {name expression} {
    upvar $name result

    set result $expression
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} [string trim $production] " " production

    set result ""
    foreach token [split $production " "] {
	if {[regexp -- {^([[:alnum:]_:]+)([?*+]|{.*})?$} $token - name quantifier]} {
	    set value [uplevel lookupValue $name]
	    append result "(?:$value)$quantifier"
	} else {
	    append result $token
	}
    }
}

source output/plaintext/documentation/grammar

puts "Popeye output grammar parser: pre-compiling"
regexp -all -indices -inline $problem::block ""

foreach inputfile $inputfiles {
    puts "Popeye output grammar parser: parsing $inputfile"
    
    set differ [open "| diff $inputfile -" "r+"]

    set f [open $inputfile "r"]
    set input [read $f]
    close $f

    set problemIndices [regexp -all -indices -inline $problem::block $input]

    foreach problemIndexPair $problemIndices {
	lassign $problemIndexPair problemStart problemEnd
	set problem [string range $input $problemStart $problemEnd]
	puts -nonewline $differ $problem
    }

    chan close $differ "write"
    
    set differences {}
    while {[gets $differ line]>=0} {
	lappend differences $line
    }

    if {[catch {
	close $differ
    } cres e]} {
	puts "differences found:"
	puts [join $differences "\n"]
    }
}
