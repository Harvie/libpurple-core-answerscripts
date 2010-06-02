#!/bin/sh
#Say it loudly and proudly!
test -x "$(which espeak)" && echo "$PURPLE_MSG" | espeak -v $(echo "$LANG" | head -c 2) &
