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
	char response[ANSWERSCRIPTS_LINE_LENGTH+1];
	FILE *pipe = job->pipe;
	PurpleConversation *conv = job->conv;

	if (pipe && !feof(pipe)) {
		if(!fgets(response, ANSWERSCRIPTS_LINE_LENGTH, pipe)
			&& (errno == EWOULDBLOCK || errno == EAGAIN) //WARNING! Not compatible with windows :-(
		) return 1;

		for(i=0;response[i];i++) if(response[i]=='\n') response[i]=0;
		purple_conv_im_send(purple_conversation_get_im_data(conv), response);

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

	/* Here are prototypes of some functions interesting to implement github feature request #3

	LOCAL USER:
	const char* purple_account_get_alias  	(  	const PurpleAccount *   	 account  	 )
	const gchar* purple_account_get_name_for_display  	(  	const PurpleAccount *   	 account  	 )
	REMOTE USER (Buddy):
	const char * 	purple_contact_get_alias (PurpleContact *contact)
	const char * 	purple_buddy_get_name (const PurpleBuddy *buddy)
	const char *	purple_buddy_get_alias_only (PurpleBuddy *buddy)
	const char * 	purple_buddy_get_server_alias (PurpleBuddy *buddy)
	const char * 	purple_buddy_get_contact_alias (PurpleBuddy *buddy)
	const char * 	purple_buddy_get_local_alias (PurpleBuddy *buddy)
	const char * 	purple_buddy_get_alias (PurpleBuddy *buddy)
	PurpleGroup * 	purple_buddy_get_group (PurpleBuddy *buddy)
	const char * 	purple_group_get_name (PurpleGroup *group)
	*/

	//Get buddy group
	const char *from_group = purple_group_get_name(purple_buddy_get_group(buddy));

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
	setenv(ENV_PREFIX "MSG", message, 1);
	setenv(ENV_PREFIX "FROM", who, 1);
	setenv(ENV_PREFIX "FROM_GROUP", from_group, 1);
	setenv(ENV_PREFIX "PROTOCOL", protocol_id, 1);
	setenv(ENV_PREFIX "STATUS", status_id, 1);
	setenv(ENV_PREFIX "STATUS_MSG", status_msg, 1);

	//Launch job on background
	answerscripts_job *job = (answerscripts_job*) malloc(sizeof(answerscripts_job));
	job->pipe = popen(hook_script, "r");
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

}

PURPLE_INIT_PLUGIN(autoanswer, init_plugin, info)
