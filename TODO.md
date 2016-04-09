#What TODO?
  * Add way to block messages (eg. based on return value?), so you can write antispam filters using answerscripts.
    * grep -o "[А-Яа-яЁё]" :-/
  * GUI for managing answerscripts with following features
    * list answerscripts
    * enable/disable answerscripts (by chmod +x/-x on them)
    * edit/create/delete answerscripts
    * install/update/share answerscripts using online git repository (aka script market)
  * Add more ANSW_ variables and features
  * Make this answerscript API stable
  * Port whole idea and plugin to other (non-purple) libraries/clients while maintaining compatibility of existing answerscripts and API
    * probably move answerscripts out of ~/.purple/ to ~/.config/answ/ or something, so they can be shared between multiple agents
  * Figure out why is first reply always followed by some ugly binary mess
    * visible in irssi
    * but it happens even for messages not sent by this plugin, so probably not my fault
