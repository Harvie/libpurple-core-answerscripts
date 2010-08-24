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
#		- On M$ Windows answerscripts.exe from libpurple directory will be called instead of this script
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
#	- To disable some of those scripts simply use: chmod -x ./script
#	- There is some basic structure, which means that all scripts should start their names with two-digit number
#	- Files are executed in order specified by those numbers and some numbers have special meanings:
#		- AB?!_ scripts without numbers are NOT executed!
#		- 00   	executed immediately, zero or single line output (parallel async processing)
#		- 01-48 executed immediately, multiline output (serial processing)
#		- 49    delay script (adds random delay to emulate human factor)
#		- 50    executed after delay, zero or single line output (parallel async processing)
#		- 51-79 executed after delay, multiline output (serial processing)
#		- 80-99	reserved for future

#legacy support, please do NOT use PURPLE_* variables in new scripts,
#this will be removed in future releases:
export PURPLE_FROM="$ANSW_FROM"
export PURPLE_MSG="$ANSW_MSG"

#this may be modified to use run-parts from coreutils in future (can't get it to work):

dir="$(dirname "$0")"; cd "$dir" #chdir to ~/.purple/ or similar
dir="${dir}/answerscripts.d"
if test -d "$dir"; then
	for i in {00..99}; do

		#sleep at 49 (this can be replaced by 49-delay.sh, but this should be faster)
		[ $i -eq 49 ] && {
			sleep $[ 2 + ($RANDOM % 8) ]; #2-9 seconds of sleep
			continue;
		}

		#execute scripts
		ls -1 "$dir/$i"* 2>/dev/null | while read script; do
			test -x "$script" && {
				#determine wheter execute on background or foreground
				if [ $i -eq 00 ] || [ $i -eq 50 ]; then
					"$script" &
				else
					"$script"
				fi;
			}
		done;

		wait; #wait for processes on background

	done;
fi
