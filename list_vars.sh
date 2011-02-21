#!/bin/bash
file='answerscripts.c'
pipe=cat
[ -n "$1" ] && pipe='sed -e s/_/\\_/g'
pre="$(grep ENV_PREFIX "$file" | head -n 1 | cut -d '"' -f 2)"
grep setenv "$file" | while read line; do
  var="$(echo "$line" | cut -d '"' -f 2)";
  wtf="$(echo "$line" | cut -d ';' -f 2 | cut -d '/' -f 3-)";
	echo -n "$1* $pre$var";
	[ -n "$wtf" ] && echo -e "\t($wtf)" || echo
done | $pipe
