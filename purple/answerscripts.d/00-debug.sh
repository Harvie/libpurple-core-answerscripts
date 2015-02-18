#!/bin/sh
#Debug
(
echo "DATE: $(date)";
env | grep -a '^ANSW_' | sort;
echo =============================================
) >&2;
