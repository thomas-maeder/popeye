namespace eval verifier {}

proc ::verifier::resolve {name} {
    variable resolved

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

proc ::verifier::init {grammarScope} {
    variable resolved
    
    set elementName "${grammarScope}::output::block"
    
    resolve $elementName

    puts "verifyer: pre-compiling"
    regexp -all -indices -inline $resolved($elementName) ""
}

proc ::verifier::treatInputFile {inputFilename content} {
    variable resolved
    
    puts "verifying $inputFilename"

    set differ [open "| diff $inputFilename -" "r+"]
    
    set parsed [regexp -inline $resolved(::grammar::output::block) $content]
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
