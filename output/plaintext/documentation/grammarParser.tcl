namespace eval ::grammarParser {
    namespace export literal terminal nonterminal lookupName scope
}

# Parse a scope and parse nested declarations
#
# @param name name of the scope
# @param nested nested declarations
proc ::grammarParser::scope {name nested} {
    uplevel namespace eval $name "namespace import ::grammarParser::*"
    uplevel namespace eval $name [list $nested]
}

# Parse the declaration of a literal, language-dependant string as part of the value of a terminal
# @param name name of the literal
#
# @return language-dependant value of the literal named $name
proc ::grammarParser::literal {name} {
    return [set ::language::${::params(language)}::$name]
}

# Parse the declaration of a terminal
# Defines a variable with the same name as the terminal.
# The value of the variable is a list consisting of
# - the string constant "terminal"
# - the terminal's value
#
# @param name name of the terminal
# @param value value of the terminal
proc ::grammarParser::terminal {name value} {
    upvar $name result

    set result [list "terminal" $value]
}

# Lookup the name of a symbol used in the value of a nonterminal
# The name is looked up
# 1. in the current scope
# 2. in the parent scope
# 3. in the grand-parent scope
# etc.
#
# @param name name to be looked up
# @param result of the lookup as a qualified name
proc ::grammarParser::lookupName {name} {
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

# Parse the declaration of a nonterminal
# Defines a variable with the same name as the nonterminal.
#
# The value of the variable is a list consisting of
# - the string constant "nonterminal"
# - a list containing one element per part of the nonterminal's value
# -- the string constant "name" or "operator"
# -- the qualified name (returned by lookupName) resp. the operator symbol
proc ::grammarParser::nonterminal {name production} {
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

# Parse the grammar into a scope
#
# @param grammarFile path to the file containing the grammar
# @param scope qualified name of the scope where the terminals,
#              nonterminals and nested scopes are added
proc ::grammarParser::parse {grammarFile scope} {
    if {[catch {
	scope $scope [list source $grammarFile]
    } error]} {
	puts $::errorInfo
	exit 1
    }
}
