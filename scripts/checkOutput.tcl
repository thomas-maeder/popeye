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

proc decomposeExpr {expr {end ""}} {
    set result {}

    while {$expr!=""} {
        set firstChar [string range $expr 0 0]
        if {$firstChar=="("} {
            lassign [decomposeExpr [string range $expr 1 "end"] ")"] nested expr
            lappend result [list "(" $nested ")"]
        } elseif {$firstChar=="\["} {
            lassign [decomposeExpr [string range $expr 1 "end"] "]"] nested expr
            lappend result [list "\[" $nested "\]"]
        } elseif {$firstChar=="\\"} {
            set escapedChar [string range $expr 0 1]
            set expr [string range $expr 2 "end"]
            lappend result $escapedChar
        } else {
            set expr [string range $expr 1 "end"]
            if {$firstChar==$end} {
                return [list $result $expr]
            } else {
                lappend result $firstChar
            }
        }
    }

    return [list $result ""]
}

proc nonterminal {name production} {
    upvar $name result

    regsub -all {[[:space:]]+} [string trim $production] " " production

    set result "(?:"
    foreach token [split $production " "] {
        switch -regexp -matchvar matches -- $token {
            ^([[:alnum:]_:]+)([?*+]|{.*})?$ {
                set name [lindex $matches 1]
                set value [uplevel lookupValue $name]
                set quantifier [lindex $matches 2]
                set decomposition [lindex [decomposeExpr $value] 0]
                if {[lsearch -exact $decomposition "|"]!=-1} {
                    append result "(?:$value)"
                } elseif {$quantifier==""} {
                    append result $value
                } elseif {[llength $decomposition]<=1} {
                    append result $value
                } else {
                    append result "(?:$value)"
                }
                append result $quantifier
            }
            default {
                append result $token
            }
        }
    }
    append result ")"
}

proc scope {name block} {
    namespace eval $name $block
}

puts "Popeye output grammar parser: preparing"
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
