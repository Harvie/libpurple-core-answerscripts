#!/bin/sh
#Debug
(
echo "DATE: $(date)";
env | grep '^ANSW_' | sort;
echo =============================================
) >&2;
