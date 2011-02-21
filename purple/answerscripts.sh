#!/bin/bash
#
#	This file is called for every message received by libpurple clients (pidgin,finch,...)
#		- You can try to rewrite this script in PERL or C for better performance (or different platform) - let me know
#		- On M$ Windows answerscripts.exe from libpurple directory will be called instead of this script
#
#	Maybe you will want to add more hooks for receiving messages, so i've made following script
#	- It just executes all +x files in answerscripts.d directory so you should do your magic there
#	- To disable some of those scripts simply use: chmod -x ./script
#	- There is some basic structure, which means that all scripts should start their names with two-digit number
#	- Files are executed in order specified by those numbers and some numbers have special meanings:
#		- AB?!_ scripts without numbers are NOT executed!
#		- 00   	executed immediately, zero or single line output (parallel async processing)
#		- 01-48 executed immediately, multiline output (serial processing)
#		- 49    delay script (adds random delay to emulate human factor, no user scripts at this level!)
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
			find "$dir"/[5-9][0-9]-* -executable | grep . >/dev/null && #check if it's worth waiting
				sleep $(( 2 + ($RANDOM % 8) )); #2-9 seconds of sleep
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
