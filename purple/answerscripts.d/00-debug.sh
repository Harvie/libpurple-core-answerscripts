#!/bin/sh
#Debug
(
echo "DATE: $(date)";
env | grep '^ANSW_';
echo =============================================
) >&2;
