#!/bin/sh
#AutoKiss :-* (just wanna be polite)
shuf="$(which shuf 2>/dev/null)"; [ -x "$shuf" ] || shuf="cat"; #shuffle if possible
echo $(echo "$ANSW_MSG" | grep -Eo ':-\*|\*IN LOVE\*|:-\{\}|\*KISSING\*' | "$shuf" );
