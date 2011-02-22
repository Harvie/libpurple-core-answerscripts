#!/usr/bin/env perl
#Just for you to see that you can use almost any language :-)
use strict;
use warnings;
use v5.10; #given/when (if you need to use older PERL, you can find older version in GIT)

$ENV{ANSW_MSG} =~ m/^([^\s]*)\s*(.*)$/;
my $args = $2;
given ($1) {
	when (/^!help$/)	{ print qx{ grep -o 'when \(/[^\$/]*' "$0" | grep -o '!.*' | tr '\n' ',' }."\n"; }
	when (/^!ping$/)	{ print "PONG"; }
	when (/^!whoami$/)	{ print "You are $ENV{ANSW_R_ALIAS} also known as $ENV{ANSW_R_NAME}"; }
	when (/^!whoareyou$/)	{ print "Hello, my name is $ENV{ANSW_L_ALIAS} ($ENV{ANSW_L_NAME}) ;-)"; }
	when (/^!version$/)	{ print "$ENV{ANSW_L_AGENT} $ENV{ANSW_L_AGENT_VERSION}"; }
	when (/^!status$/)	{ print "[$ENV{ANSW_L_STATUS}] $ENV{ANSW_L_STATUS_MSG}"; }
	when (/^!(reboot|reset|restart|halt)$/)	{ print "Broadcast message: The system is going down for reboot NOW !!"; }
	when (/^!google$/)	{ my $q = $args; $q =~ s/ /+/g; print "UTFG Yourself: http://google.com/search?q=$q"; }
	when (/^!uptime$/)	{ print qx{uptime}; }
	when (/^!date$/)	{ print qx{date}; }
	when (/^!dmesg$/)	{ print qx{ dmesg | tail -n 5 | tr '\n' '\t' } }
	when (/^!df$/)	{ print qx{ df -hlP / | tail -n 1 } }
}
