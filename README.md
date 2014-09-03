fast
====

terminal fast reader for piped input (like more or less)

dependencies: 

    libncurses

compile:

    gcc -o fast fast.c -lncurses

usage:

    cat file | fast
