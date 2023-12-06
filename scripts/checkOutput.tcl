#! /usr/bin/env tclsh

source output/plaintext/documentation/german
source output/plaintext/documentation/english
source output/plaintext/documentation/cmdline.tcl

set options {
    { language.arg "english" {(-l) output language} }
    { l.arg "english" {} }
}

set usageIntro [join {
    {: [options] filepath ...}
    {}
    {options:}
} "\n"]

try {
    array set params [::cmdline::getoptions argv $options $usageIntro]
    # Note: argv is modified now. The recognized options are
    # removed from it, leaving the non-option arguments behind.
} trap {CMDLINE USAGE} {msg o} {
    # Trap the usage signal, print the message, and exit the application.
    # Note: Other errors are not caught and passed through to higher levels!
    puts $msg
    exit 1
}

set params(inputfiles) $argv

proc literal {name} {
    return [set ${::params(language)}::$name]
}

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

    set result [list "terminal" $expression]
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} [string trim $production] " " production

    set lookedup ""
    foreach token [split $production " "] {
	if {[regexp -- {^([[:alnum:]_:]+)([?*+]|{.*})?$} $token - name quantifier]} {
	    set name [uplevel lookupName $name]
	    lappend lookedup [list "name" $name $quantifier]
	} else {
	    lappend lookedup [list "operator" $token ""]
	}
    }

    set result [list "nonterminal" $lookedup]
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
