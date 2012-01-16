#!/bin/sh
#Artificial Pseudo Inteligence using megahal package :-)
#More info: http://megahal.alioth.debian.org/
#alias cat=true #silent mode, comment out to enable loud replies :-)
echo -n '[MegaHAL]:' $({ echo "$ANSW_MSG" &&	echo -e '\n#quit\n';	} | megahal -p -b -w | tail -n1) | cat
