#!/bin/sh
#This script will forward all incoming IM messages to someone else in network
FORWARD_PROTOCOL='irc';
FORWARD_ACCOUNT='Harvie@irc.freenode.net';
FORWARD_TO='hrv';
purple-remote "$FORWARD_PROTOCOL:goim?account=$FORWARD_ACCOUNT&screenname=$FORWARD_TO&message=""$(echo "<$ANSW_PROTOCOL:$ANSW_R_NAME> $ANSW_MSG" | tr '&' ' ')"
