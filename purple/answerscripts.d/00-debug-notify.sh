#!/bin/sh
#Debug for systems with notify
notify-send AnswerScripts "$(env | grep -a '^ANSW_' | sort)" &>/dev/null
