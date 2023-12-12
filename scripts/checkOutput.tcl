#! /usr/bin/env tclsh

source output/plaintext/documentation/german
source output/plaintext/documentation/english
source output/plaintext/documentation/cmdline.tcl
source output/plaintext/documentation/grammarParser.tcl

set languagesRE [join [regsub -all ::language:: [namespace children ::language] ""] "|"]

set options [subst -nocommands -nobackslashes {
    { language.arg "english" "output language ($languagesRE)" }
}]

set usageIntro [join {
    {: [options] filepath ...}
    {}
    {options:}
} "\n"]

try {
    array set params [::cmdline::getoptions argv $options $usageIntro]

    if {[regexp -- $languagesRE $params(language)]} {
	# Note: argv is modified now. The recognized options are
	# removed from it, leaving the non-option arguments behind.
	set params(inputfiles) $argv
    } else {
	::cmdline::Error [::cmdline::usage $options $usageIntro] USAGE
    }
} trap {CMDLINE USAGE} {msg o} {
    # Trap the usage signal, print the message, and exit the application.
    # Note: Other errors are not caught and passed through to higher levels!
    puts $msg
    exit 1
}

::grammarParser::parse output/plaintext/documentation/grammar ::grammar

proc resolve {name} {
    global resolved

    if {![info exists resolved($name)]} {
	lassign [set $name] type value
	switch $type {
	    "terminal" {
		set resolved($name) $value
	    }
	    "nonterminal" {
		foreach element $value {
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
    }
}

resolve "::grammar::output::block"


puts "Popeye output grammar parser: pre-compiling"
regexp -all -indices -inline $resolved(::grammar::output::block) ""

foreach inputfile $params(inputfiles) {
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
