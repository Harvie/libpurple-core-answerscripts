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
#include <libpurple/debug.h>
#include <libpurple/version.h>
#include <libpurple/conversation.h>
#include <libpurple/plugin.h>
#include <libpurple/signals.h>
#include <libpurple/util.h>

char *message = NULL;
char *hook_script = NULL;

typedef struct {
  FILE *pipe;
  PurpleConversation *conv;
} answerscripts_job;

int answerscripts_process_message_cb(answerscripts_job *job) {
	int i;
	char response[ANSWERSCRIPTS_LINE_LENGTH+1]; response[0]='\0';
	FILE *pipe = job->pipe;
	PurpleConversation *conv = job->conv;

	if (pipe && !feof(pipe)) {
		if(!fgets(response, ANSWERSCRIPTS_LINE_LENGTH, pipe)
			&& (errno == EWOULDBLOCK || errno == EAGAIN) //WARNING! Not compatible with windows :-(
		) return 1;

		for(i=0;response[i];i++) if(response[i]=='\n') response[i]=0;
		if(response[0]!='\0') purple_conv_im_send(purple_conversation_get_im_data(conv), response);

		if(!feof(pipe)) return 1;
	}
	pclose(pipe);
	free(job);
	return 0;
}

static void received_im_msg_cb(PurpleAccount *account, char *who, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data) {
	if (conv == NULL) conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, who); //* A workaround to avoid skipping of the first message as a result on NULL-conv: */
	PurpleBuddy *buddy = purple_find_buddy(account, who);

	//Get message
	message = purple_markup_strip_html(buffer);

	//Here are prototypes of some functions interesting to implement github feature request #3

	//LOCAL USER:
	const char* local_alias = purple_account_get_alias(account);
	const char* local_name = (char *) purple_account_get_name_for_display(account);
	setenv(ENV_PREFIX "LOCAL_NAME", local_name, 1);	//Name of local user - untested
	setenv(ENV_PREFIX "LOCAL_ALIAS", local_alias, 1);	//Alias of local user - untested

	//REMOTE USER (Buddy):
	//const char * 	purple_contact_get_alias (PurpleContact *contact)
	const char* remote_name = purple_buddy_get_name(buddy);
	const char* remote_alias_only = purple_buddy_get_alias_only(buddy);
	const char* remote_server_alias = purple_buddy_get_server_alias(buddy);
	const char* remote_contact_alias = purple_buddy_get_contact_alias(buddy);
	const char* remote_local_alias = purple_buddy_get_local_alias(buddy);
	const char* remote_alias = purple_buddy_get_alias(buddy);
	setenv(ENV_PREFIX "REMOTE_NAME", remote_name, 1);	//???
	setenv(ENV_PREFIX "REMOTE_ALIAS_ONLY", remote_alias_only, 1);	//buggy
	setenv(ENV_PREFIX "REMOTE_SERVER_ALIAS", remote_server_alias, 1);	//buggy
	setenv(ENV_PREFIX "REMOTE_CONTACT_ALIAS", remote_contact_alias, 1);	//buggy
	setenv(ENV_PREFIX "REMOTE_LOCAL_ALIAS", remote_local_alias, 1);	//???
	setenv(ENV_PREFIX "REMOTE_ALIAS", remote_alias, 1);	//???

	//Get buddy group
	PurpleGroup *group = purple_buddy_get_group(buddy);
	const char *from_group = group != NULL ? purple_group_get_name(group) : ""; //return empty string if not in group

	//Get protocol ID
	const char *protocol_id = purple_account_get_protocol_id(account);
	if(!strncmp(protocol_id,PROTOCOL_PREFIX,strlen(PROTOCOL_PREFIX))) protocol_id += strlen(PROTOCOL_PREFIX); //trim out PROTOCOL_PREFIX (eg.: "prpl-irc" => "irc")

	//Get status
	PurpleStatus *status = purple_account_get_active_status(account);
	PurpleStatusType *type = purple_status_get_type(status);

	//Get status id
	const char *status_id = NULL;
	status_id = purple_primitive_get_id_from_type(purple_status_type_get_primitive(type));

	//Get status message
	const char *status_msg = NULL;
	if (purple_status_type_get_attr(type, "message") != NULL) {
		status_msg = purple_status_get_attr_string(status, "message");
	} else {
		status_msg = (char *) purple_savedstatus_get_message(purple_savedstatus_get_current());
	}

	//Export variables to environment
	setenv(ENV_PREFIX "MSG", message, 1);	//text of the message
	setenv(ENV_PREFIX "FROM", who, 1);	//who sent you the message
	setenv(ENV_PREFIX "FROM_GROUP", from_group, 1);	//group which contains that buddy
	setenv(ENV_PREFIX "PROTOCOL", protocol_id, 1);	//protocol used to deliver the message. eg.: xmpp, irc,...
	setenv(ENV_PREFIX "STATUS", status_id, 1);	//unique ID of status. eg.: available, away,...
	setenv(ENV_PREFIX "STATUS_MSG", status_msg, 1);	//status message set by user

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
	purple_signal_connect(conv_handle, "received-im-msg", plugin, PURPLE_CALLBACK(received_im_msg_cb), NULL);
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
	"0.3.1",
	"Framework for hooking scripts to process received messages for libpurple clients",
	"\nThis plugin will execute script \"~/.purple/" ANSWERSCRIPT "\" "
		"(or any other executable called \"" ANSWERSCRIPT "\" and found in purple_user_dir()) "
		"each time when instant message is received.\n"
		"\n- Any text printed to STDOUT by this script will be sent back as answer to received message."
		"\n- Following environment values will be set, so script can use them for responding:\n"
		"\t- ANSW_MSG\n"
		"\t- ANSW_FROM\n"
		"\t- ANSW_PROTOCOL\n"
		"\t- ANSW_STATUS\n"
		"\t- ANSW_STATUS_MSG\n"
		"\nPlease see sample scripts, documentation, website and source code for more informations...\n"
		"\n(-; Peace ;-)\n",
	"Tomas Mudrunka <harvie@email.cz>",
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
	setenv(ENV_PREFIX "AGENT", (char *) purple_core_get_ui(), 1);	//ID of IM client used with answerscripts
	setenv(ENV_PREFIX "AGENT_VERSION", (char *) purple_core_get_version(), 1);	//Version of client
}

PURPLE_INIT_PLUGIN(autoanswer, init_plugin, info)
