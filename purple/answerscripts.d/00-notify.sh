#!/bin/sh
#Show notification for each incomming message
notify-send -i pidgin "$ANSW_PROTOCOL:$ANSW_R_NAME" "$(echo "$ANSW_MSG" | sed -e 's/&/&amp;/g')"
