#!/bin/sh
#Show notification for each incomming message
which recode >/dev/null && ANSW_MSG="$(echo "$ANSW_MSG" | recode ascii..html)"
notify-send -i pidgin "$ANSW_PROTOCOL:$ANSW_R_NAME" "$ANSW_MSG"
