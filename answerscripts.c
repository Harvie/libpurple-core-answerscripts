#define _GNU_SOURCE
//#define __WIN32__
#ifndef __WIN32__
	#define ANSWERSCRIPT_EXT ""
#else
	#define ANSWERSCRIPT_EXT ".exe"
#endif
#define ANSWERSCRIPT "answerscripts" ANSWERSCRIPT_EXT
#define ANSWERSCRIPTS_TIMEOUT_INTERVAL 250
#define ANSWERSCRIPTS_LINE_LENGTH 4096
#define ENV_PREFIX "ANSW_"
#define PROTOCOL_PREFIX "prpl-"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef __WIN32__
	#include <fcntl.h>
#else
	#include <windows.h>
#endif

/* Purple plugin */
#define PURPLE_PLUGINS
#include <libpurple/account.h>
#include <libpurple/blist.h>
#include <libpurple/conversation.h>
#include <libpurple/core.h>
#include <libpurple/debug.h>
#include <libpurple/plugin.h>
#include <libpurple/savedstatuses.h>
#include <libpurple/signals.h>
#include <libpurple/status.h>
#include <libpurple/util.h>
#include <libpurple/value.h>
#include <libpurple/version.h>

char *message = NULL;
char *hook_script = NULL;

typedef struct {
  FILE *pipe;
  PurpleConversation *conv;
} answerscripts_job;

const void *check_null(const void *pointer) {
	if(pointer == NULL) return "";
	return pointer;
}

int answerscripts_process_message_cb(answerscripts_job *job) {
	int i;
	char response[ANSWERSCRIPTS_LINE_LENGTH+1]; response[0]='\0';
	FILE *pipe = job->pipe;
	PurpleConversation *conv = job->conv;

	if (pipe && !feof(pipe)) {
		if(!fgets(response, ANSWERSCRIPTS_LINE_LENGTH, pipe)
			&& (errno == EWOULDBLOCK || errno == EAGAIN) //WARNING! Not compatible with windows :-(
		) return 1;

		for(i=0;response[i];i++) if(response[i]=='\n') response[i]='\0';
		if(response[0]!='\0') {
			if(purple_conversation_get_type(conv) == PURPLE_CONV_TYPE_CHAT) {
				purple_conv_chat_send(purple_conversation_get_chat_data(conv), response);
			} else {
				purple_conv_im_send(purple_conversation_get_im_data(conv), response);
			}
		}
		if(!feof(pipe)) return 1;
	}
	pclose(pipe);
	free(job);
	return 0;
}

static void received_msg_cb(PurpleAccount *account, char *who, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data) {
	if (conv == NULL) conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, who); //* A workaround to avoid skipping of the first message as a result on NULL-conv: */

	PurpleBuddy *buddy = purple_find_buddy(account, who);
	PurplePresence *presence = purple_buddy_get_presence(buddy);

	//Get message
	message = purple_markup_strip_html(buffer);

	//Get conversation type
	const char *action, *roomname;
	switch(purple_conversation_get_type(conv)) {
		case PURPLE_CONV_TYPE_IM:
			action = "IM";
			roomname = "";
			break;
		case PURPLE_CONV_TYPE_CHAT:
			action = "CHAT";
			roomname = purple_conversation_get_name(conv);
			//PurpleConvChat *chat = purple_conversation_get_chat_data(conv);
			break;
		default:
			action = "UNKNOWN";
	}

	//LOCAL USER:
	const char* local_name = (char *) purple_account_get_name_for_display(account);
	const char* local_alias = purple_account_get_alias(account);
	if(local_alias == NULL) local_alias = local_name;

	//Do not respond to messages sent by myself
	if(strcmp(local_name, who) == 0) return;

	//Was my nick said?
	char *highlighted;
	if(flags & PURPLE_MESSAGE_NICK || purple_utf8_has_word(buffer, local_name))
		highlighted = "true";
	else
		highlighted = "false";

	//REMOTE USER (Buddy):

	//Get buddy alias
	const char* remote_alias = purple_buddy_get_alias(buddy);
	if(remote_alias == NULL) remote_alias = who;
	//if(remote_alias == NULL) remote_alias = "";

	//Get buddy group
	PurpleGroup *group = purple_buddy_get_group(buddy);
	const char *from_group = group != NULL ? purple_group_get_name(group) : ""; //return empty string if not in group

	//Get protocol ID
	const char *protocol_id = purple_account_get_protocol_id(account);
	if(!strncmp(protocol_id,PROTOCOL_PREFIX,strlen(PROTOCOL_PREFIX))) protocol_id += strlen(PROTOCOL_PREFIX); //trim out PROTOCOL_PREFIX (eg.: "prpl-irc" => "irc")

	//Get status
	PurpleStatus *status = purple_account_get_active_status(account);
	PurpleStatusType *type = purple_status_get_type(status);
	//remote
	PurpleStatus *r_status = purple_presence_get_active_status(presence);
	PurpleStatusType *r_status_type =	purple_status_get_type(r_status);

	//Get status id
	const char *status_id = NULL;
	status_id = purple_primitive_get_id_from_type(purple_status_type_get_primitive(type));
	//remote
	const char *r_status_id = NULL;
	r_status_id = purple_primitive_get_id_from_type(purple_status_type_get_primitive(r_status_type));

	//Get status message
	const char *status_msg = NULL;
	if (purple_status_type_get_attr(type, "message") != NULL) {
		status_msg = check_null(purple_status_get_attr_string(status, "message"));
	} else {
		status_msg = (char *) check_null(purple_savedstatus_get_message(purple_savedstatus_get_current()));
	}
	//remote
	const char *r_status_msg = NULL;
	if (purple_status_type_get_attr(r_status_type, "message") != NULL) {
		r_status_msg = check_null(purple_status_get_attr_string(r_status, "message"));
	} else {
		r_status_msg = "";
	}

	//Export variables to environment
	setenv(ENV_PREFIX "ACTION", action, 1);	//what happend: IM/CHAT/UNKNOWN, show setting dialog, event, etc...
	setenv(ENV_PREFIX "MSG", message, 1);	//text of the message
	setenv(ENV_PREFIX "MSG_HIGHLIGHTED", highlighted, 1);	//was my nick mentioned in message? true/false
	setenv(ENV_PREFIX "PROTOCOL", protocol_id, 1);	//protocol used to deliver the message. eg.: xmpp, irc,...
	setenv(ENV_PREFIX "R_NAME", who, 1);	//ID of remote user - "buddy"
	setenv(ENV_PREFIX "R_GROUP", from_group, 1);	//group which contains that buddy OR empty string
	setenv(ENV_PREFIX "R_ALIAS", remote_alias, 1);	//buddy's OPTIONAL alias, server alias, contact alias, username OR empty string
	setenv(ENV_PREFIX "R_STATUS", r_status_id, 1);	//unique ID of remote user's status. eg.: available, away,...
	setenv(ENV_PREFIX "R_ROOM_NAME", roomname, 1);	//Chatroom name
	setenv(ENV_PREFIX "R_STATUS_MSG", r_status_msg, 1);	//status message set by your buddy
	setenv(ENV_PREFIX "L_NAME", local_name, 1);	//ID of local user
	setenv(ENV_PREFIX "L_ALIAS", local_alias, 1);	//OPTIONAL alias of local user OR empty string
	setenv(ENV_PREFIX "L_STATUS", status_id, 1);	//unique ID of local user's status. eg.: available, away,...
	setenv(ENV_PREFIX "L_STATUS_MSG", status_msg, 1);	//status message set by local user

	//Launch job on background
	answerscripts_job *job = (answerscripts_job*) malloc(sizeof(answerscripts_job));
	job->pipe = popen(hook_script, "r");
	if(job->pipe == NULL) {
		fprintf(stderr,"Can't execute %s\n", hook_script);
		return;
	}
	job->conv = conv;

	#ifndef __WIN32__
		int fflags = fcntl(fileno(job->pipe), F_GETFL, 0);
		fcntl(fileno(job->pipe), F_SETFL, fflags | O_NONBLOCK);
	#else
		//WARNING! Somehow implement FILE_FLAG_OVERLAPPED & FILE_FLAG_NO_BUFFERING support on windows
	#endif

	purple_timeout_add(ANSWERSCRIPTS_TIMEOUT_INTERVAL, (GSourceFunc) answerscripts_process_message_cb, (gpointer) job);
}

static gboolean plugin_load(PurplePlugin * plugin) {
	asprintf(&hook_script,"%s/%s",purple_user_dir(),ANSWERSCRIPT);
	void *conv_handle = purple_conversations_get_handle();
	purple_signal_connect(conv_handle, "received-im-msg", plugin, PURPLE_CALLBACK(received_msg_cb), NULL);
	purple_signal_connect(conv_handle, "received-chat-msg", plugin, PURPLE_CALLBACK(received_msg_cb), NULL);
	return TRUE;
}

static gboolean plugin_unload(PurplePlugin * plugin) {
	free(hook_script);
	return TRUE;
}

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,

	"core-answerscripts",
	"AnswerScripts",
	"0.5.3",
	"Framework for hooking scripts to process received messages for libpurple clients",
	"\nThis plugin will execute script \"~/.purple/" ANSWERSCRIPT "\" "
		"(or any other executable called \"" ANSWERSCRIPT "\" and found in purple_user_dir()) "
		"each time when instant message is received.\n"
		"\n- Any text printed to STDOUT by this script will be sent back as answer to received message."
		"\n- Following environment values will be set, so script can use them for responding:\n"
		"\t- " ENV_PREFIX "* (see documentation or env for more)\n"
		"\nPlease see sample scripts, documentation, website and source code for more informations...\n"
		"\n(-; Peace ;-)\n",
	"Tomas Mudrunka <harviecz@gmail.cz>",
	"http://github.com/harvie/libpurple-core-answerscripts",

	plugin_load,
	plugin_unload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

static void init_plugin(PurplePlugin * plugin) {
	//Export static environment variables
	#ifndef __x86_64__ //Workaround for x86_64 (where this causes problems for unknown reason)
		const char * core_ui = check_null(purple_core_get_ui());
		const char * core_version = check_null(purple_core_get_version());
		setenv(ENV_PREFIX "L_AGENT", (char *) core_ui, 1);	//ID of IM client used with answerscripts
		setenv(ENV_PREFIX "L_AGENT_VERSION", (char *) core_version, 1);	//Version of client
	#endif
}

PURPLE_INIT_PLUGIN(autoanswer, init_plugin, info)
