#!/bin/sh

#	This file is called for every message received by libpurple clients (pidgin,finch,...)
#		- Following env values are passed to this script:
#			- ANSW_FROM	(who sent you message)
#			- ANSW_MSG	(text of the message)
#			- ANSW_STATUS	(unique ID of status. eg.: available, away,...)
#			- ANSW_STATUS_MSG	(status message set by user)
#		- WARNING: You should mind security (don't let attackers to execute their messages/nicks!)
#		- Each line of output is sent as reply to that message
#		- You can try to rewrite this script in PERL or C for better performance (or different platform)
#		- This script have .exe suffix as i hope it can be eventualy replaced by some binary on windows
#
#	Basic example can look like this:
#		[ "$ANSW_STATUS" != 'available' ] && echo "<$ANSW_FROM> $ANSW_MSG" && echo "My status: $ANSW_STATUS_MSG";
#
#	There are lot of hacks that you can do with this simple framework if you know some scripting. eg.:
#	- Forward your instant messages to email, SMS gateway, text-to-speach (eg. espeak) or something...
#		- Smart auto-replying messages based on regular expressions
#		- Remote control your music player (or anything else on your computer) using instant messages
#	- Simple IRC/Jabber/ICQ bot (accepts PM only, you can run finch in screen on server)
#	- Providing some service (Searching web, Weather info, System status, RPG game...)
#	- BackDoor (even unintentional one - you've been warned)
#	- Loging and analyzing messages
#	- Connect IM with Arduino
#	- Annoy everyone with spam (and probably get banned everywhere)
#	- Anything else that you can imagine...
#
#	Maybe you will want to add more hooks for receiving messages, so i've made following script
#	- It just executes all +x files in answerscripts.d directory so you should do your magic there
#	- To disable some of those scripts just use chmod -x script

#legacy support, please do NOT use PURPLE_* variables in new scripts,
#this will be removed in future releases:
export PURPLE_FROM="$ANSW_FROM"
export PURPLE_MSG="$ANSW_MSG"

#this should be modified to use run-parts in future:
dir="$(dirname "$0")"/answerscripts.d
if test -d "$dir"; then
	for script in "$dir"/*; do
		test -x "$script" && "$script"
	done
fi
