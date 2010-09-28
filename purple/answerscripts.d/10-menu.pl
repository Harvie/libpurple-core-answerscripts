#!/usr/bin/env perl
#Just for you to see that you can use almost any language :-)
use strict;
use warnings;
use v5.10; #given/when (if you need to use older PERL, you can find older version in GIT)

given ($ENV{ANSW_MSG}) {
	when (/^!help$/)	{ print qx{ grep -o 'when \(/[^\$/]*' "$0" | grep -o '!.*' | tr '\n' ',' }; }
	when (/^!ping$/)	{ print "PONG"; }
	when (/^!whoami$/)	{ print "You are: $ENV{ANSW_FROM}"; }
	when (/^!status$/)	{ print "[$ENV{ANSW_STATUS}] $ENV{ANSW_STATUS_MSG}"; }
	when (/^!(reboot|reset|restart|halt)$/)	{ print "Broadcast message: The system is going down for reboot NOW !!"; }
	when (/^!google/)	{ print "UTFG Yourself: http://google.com/"; }
	when (/^!uptime$/)	{ print qx{uptime}; }
	when (/^!date$/)	{ print qx{date}; }
	when (/^!dmesg$/)	{ print qx{ dmesg | tail -n 5 | tr '\n' '\t' } }
}
