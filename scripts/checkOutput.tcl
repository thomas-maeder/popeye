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
proc lookupName {name} {
    set scope [uplevel namespace current]
    set initialscope $scope
    while {![info exists ${scope}::$name]} {
        if {$scope=="::grammar"} {
            error "can't find variable $name from scope $initialscope"
        } else {
            set scope [namespace parent $scope]
        }
    }
    return ${scope}::$name
}

proc terminal {name expression} {
    upvar $name result
    global resolved

    set result $expression
    set resolved([uplevel namespace current]::$name) $expression
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} [string trim $production] " " production

    set result ""
    foreach token [split $production " "] {
	if {[regexp -- {^([[:alnum:]_:]+)([?*+]|{.*})?$} $token - name quantifier]} {
	    set name [uplevel lookupName $name]
	    lappend result [list "name" $name $quantifier]
	} else {
	    lappend result [list "operator" $token ""]
	}
    }
}

if {[catch {
    namespace eval grammar {
	source output/plaintext/documentation/grammar
    }
} error]} {
    puts $errorInfo
    exit 1
}

proc resolve {name} {
    global resolved

    if {![info exists resolved($name)]} {
	if {![info exists $name]} {
	    error "name $name is not defined"
	}
	foreach element [set $name] {
	    lassign $element type value quantifier
	    switch $type {
		"name" {
		    resolve $value
		    append resolved($name) "(?:$resolved($value))$quantifier"
		}
		"operator" {
		    append resolved($name) $value
		}
	    }
	}
    }
}

resolve "::grammar::output::block"


puts "Popeye output grammar parser: pre-compiling"
regexp -all -indices -inline $resolved(::grammar::output::block) ""

foreach inputfile $inputfiles {
    puts "Popeye output grammar parser: parsing $inputfile"
    
    set differ [open "| diff $inputfile -" "r+"]

    set f [open $inputfile "r"]
    set input [read $f]
    close $f

    set parsed [regexp -inline $resolved(::grammar::output::block) $input]
    puts -nonewline $differ [lindex $parsed 0]

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
