#!/bin/sh
#Debug to pidgin's STDERR
(
echo "DATE: $(date)";
env | grep -a '^ANSW_' | sort;
echo =============================================
) >&2;
