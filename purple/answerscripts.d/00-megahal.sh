#!/bin/sh
#Artificial Pseudo Inteligence using megahal package :-)
#More info: http://megahal.alioth.debian.org/

AI_LOUD=false #enable loud replies (otherwise will be learning only :-)
AI_LOUD_UNAVAILABLE=true; #enable loud replies when not available :-)
AI_REMEMBER=true; #set false if you don't want MegaHAL to learn new phrases (this can prevent data corruption)

$AI_LOUD_UNAVAILABLE && [ "$ANSW_L_STATUS" != 'available' ] && AI_LOUD=true
$AI_LOUD || $AI_REMEMBER || exit 23; #No speaking + No learning = Exitus

AI_CMD_QUIT='#EXIT'
$AI_REMEMBER && AI_CMD_QUIT='#QUIT'
AI_ANSW="$({ echo "$ANSW_MSG." | sed -e 's/#//g' && echo -e "\n$AI_CMD_QUIT\n"; } | timeout 23 megahal -p -b -w | tail -n1)"
$AI_LOUD && [ -n "$AI_ANSW" ] && echo -n "[MegaHALuz]: $AI_ANSW"
