#!/usr/bin/env perl
#Just for you to see that you can use almost any language :-)
use strict;
use warnings;
use Switch;

switch ($ENV{ANSW_MSG}) {
	case /^!help$/	{ print qx{ grep -o 'case /[^\$/]*' "$0" | grep -o '!.*' | tr '\n' ',' }; }
	case /^!ping$/	{ print "PONG"; }
	case /^!whoami$/	{ print "You are: $ENV{ANSW_FROM}"; }
	case /^!status$/	{ print "[$ENV{ANSW_STATUS}] $ENV{ANSW_STATUS_MSG}"; }
	case /^!(reboot|reset|restart|halt)$/	{ print "Broadcast message: The system is going down for reboot NOW !!"; }
	case /^!google/	{ print "UTFG Yourself: http://google.com/"; }
	case /^!uptime$/	{ print qx{uptime}; }
	case /^!date$/	{ print qx{date}; }
	case /^!dmesg$/	{ print qx{ dmesg | tail -n 5 | tr '\n' '\t' } }
}
