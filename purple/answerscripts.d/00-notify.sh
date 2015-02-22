#!/bin/sh
#Show notification for each incomming message
ANSW_MSG="$(echo "$ANSW_MSG" | sed -e 's/&/&amp\;/g;s/</\&lt\;/g;s/>/\&gt;/g')"
notify-send -i pidgin "$ANSW_PROTOCOL:$ANSW_R_NAME" "$ANSW_MSG"
