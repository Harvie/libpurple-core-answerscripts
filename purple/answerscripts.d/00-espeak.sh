#!/bin/sh
#Say it loudly and proudly!
test -x "$(which espeak)" && echo "$ANSW_MSG" | espeak -v $(echo "$LANG" | head -c 2) &
