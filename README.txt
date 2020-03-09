                                    joblog

A command line tool to track your work.

To compile, clone the repository and compile 'joblog.cpp' using your preferred
C++ compiler. Rename your result and move it somewhere it is found by your
system.


Useage: joblog [--version] [--help] [-<args>] <command> [<args>]

Commands:

help    Print this help message or further help on a topic.
init    Initialize a logfile.
start   Begin working.
end     End working.
log     Write down what you did.
state   Give a short overview of the current state.
list    List what was done.

Use 'joblog help <topic>' to get further help on a topic.
Available topics are: start, end, list, args
