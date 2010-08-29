#libPurple core-answerscripts plugin
  * Framework for hooking scripts to **respond received messages** (and maybe bit more in future) for various **libpurple** clients such as **pidgin or finch**
  * This simple plugin just passes every single message received by any libPurple-based client (pidgin,finch) to sript(s) in user's home directory... So **you can add various hooks.**
  * There are already few sample (answer)scripts in ./purple directory, so you can check how easy it is to write some script for pidgin or finch...

##What can this do for me?

There are lot of hacks that you can do with this simple framework if you know some basic scripting. eg.:

- **Forward your instant messages** to email, SMS gateway, text-to-speach (eg. espeak) or something...
  - Smart auto-replying messages based on regular expressions
  - Remote control your music player (or anything else on your computer) using instant messages
- Simple IRC/Jabber/ICQ bot (currently accepts PM only, you can run finch in screen on server)
- Providing some service (Searching web, Weather info, System status, RPG game...)
- BackDoor (**even unintentional one - you've been warned**)
- Loging and analyzing messages
- Connect IM with Arduino
- Annoy everyone with spam (and probably **get banned everywhere**)
- **Anything else that you can imagine...** (i'm looking forward to hearing your stories)

##Writing own (answer)scripts

  * Check example scripts in **./purple/answerscripts.d/** to see how easy it is
  * Basically
    * Each time you receive message, the main **answerscripts.sh script (answerscripts.exe on M$ Windows) is executed** on background
    * Every line that is outputed by this script to it's **STDOUT is sent** as response to message that executed it
    * Following **environment values are passed** to the script:
      * ANSW\_MSG	(text of the message)
      * ANSW\_FROM	(who sent you message)
      * ANSW\_PROTOCOL	(protocol used to deliver the message. eg.: jabber, irc,...)
      * ANSW\_STATUS	(unique ID of status. eg.: available, away,...)
      * ANSW\_STATUS\_MSG	(status message set by user)
    * **WARNING: You should mind security (don't let attackers to execute their messages/nicks!)**
    * I guess that you will want to use more than one answerscript, so i made such answerscript which will execute all answerscripts in **~/.purple/answerscripts.d**
      * It's quite smart and all you need to do is set the filenames and permissions of answerscripts in that directory properly...
      * See it's (**./purple/answerscripts.sh**) comments for rest of documentation...

###Example
Following answerscript will reply to each incoming message if you are not available. Reply will consist of two messages: one with username of your buddy who sent you a message and text of that message; and second with your status message. Simple huh?

    #!/bin/sh
    [ "$ANSW_STATUS" != 'available' ] && echo "<$ANSW_FROM> $ANSW_MSG" && echo "My status: $ANSW_STATUS_MSG";

##Building & installation

###From packages
- ArchLinux: http://aur.archlinux.org/packages.php?ID=37942
###Manually
- The libpurple header files are needed to compile the plugin.
- To build and install :
  You can compile the plugin using

        $ make

  and install it with

        $ make install

  This will install it in ~/.purple/plugins so that only the user who install it can use it.

        $ make user

  Install main script and sample answerscripts to ~/.purple/answerscripts.d/

- To install it for everybody on your computer,

        $ make
        $ su
        # make install PREFIX="/path/to/libpurple" (this command as root user)

  generally /path/to/libpurple is /usr or /usr/local. If you don't know the path then you can find out using

        $ whereis libpurple

  and look for the part before "/lib/libpurple.so".
