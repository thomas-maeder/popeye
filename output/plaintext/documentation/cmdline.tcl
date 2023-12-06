package require cmdline

namespace eval cmdline {
    rename usage usage_orig
}

proc ::cmdline::usage {optlist {usage {options:}}} {
    set result [usage_orig $optlist $usage]

    # suppress lines with 1 letter options (i.e. -e, but not -?)
    # they are shortcuts and better documented along with the regular long named option
    regsub -all -line {^ -[[:alpha:]] [^\n]+\n} $result {} result

    # use form --opt=val for options with long name and value
    regsub -all -line {^ -([[:alpha:]]+) value } $result { --\1=value} result

    # use form --opt for options with long name and no value
    regsub -all -line {^ -([[:alpha:]]+) } $result { --\1} result
    
    return $result
}

# original getArgv0 removes the suffix for no particular reason
proc ::cmdline::getArgv0 {} {
    return [file tail $::argv0]
}
