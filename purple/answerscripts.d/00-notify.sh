#!/bin/sh
#Show notification for each incomming message
ANSW_MSG="$(echo "$ANSW_MSG" | sed -e 's/&/&amp\;/g;s/</\&lt\;/g;s/>/\&gt;/g')"
[ -n "$ANSW_R_ROOM_NAME" ] && room="$ANSW_R_ROOM_NAME:" || room=''
notify-send -i pidgin "$ANSW_PROTOCOL:$room$ANSW_R_ALIAS" "$ANSW_MSG"
