namespace eval ::grammarParser {
    namespace export literal terminal nonterminal lookupName scope
}

proc ::grammarParser::scope {name code} {
    uplevel namespace eval $name "namespace import ::grammarParser::*"
    uplevel namespace eval $name [list $code]
}

proc ::grammarParser::literal {name} {
    return [set ::language::${::params(language)}::$name]
}

proc ::grammarParser::terminal {name expression} {
    upvar $name result

    set result [list "terminal" $expression]
}

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

proc ::grammarParser::parse {grammarFile scope} {
    if {[catch {
	scope $scope [list source $grammarFile]
    } error]} {
	puts $::errorInfo
	exit 1
    }
}
