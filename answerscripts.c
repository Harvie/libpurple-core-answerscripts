//#define __WIN32__
#ifndef __WIN32__
	#define ANSWERSCRIPT_EXT ""
#else
	#define ANSWERSCRIPT_EXT ".exe"
#endif
#define ANSWERSCRIPT "answerscripts" ANSWERSCRIPT_EXT
#define ANSWERSCRIPTS_TIMEOUT_INTERVAL 250
#define ANSWERSCRIPTS_LINE_LENGTH 4096

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef __WIN32__
	#include <fcntl.h>
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
char response[ANSWERSCRIPTS_LINE_LENGTH+1];
int i;

typedef struct {
  FILE *pipe;
  PurpleConversation *conv;
} answerscripts_job;

int answerscripts_process_message_cb(answerscripts_job *job) {
	FILE *pipe = job->pipe;
	PurpleConversation *conv = job->conv;

	if (pipe && !feof(pipe)) {
		if(!fgets(response, ANSWERSCRIPTS_LINE_LENGTH, pipe)
			&& (errno == EWOULDBLOCK || errno == EAGAIN)
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

	//Get message
	message = purple_markup_strip_html(buffer);
	//printf("\nHarvie received: %s: %s\n", who, message); //debug
	//purple_conv_im_send(purple_conversation_get_im_data(conv), ":-*"); //debug

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
	setenv("PURPLE_FROM", who, 1);
	setenv("PURPLE_MSG", message, 1);
	setenv("PURPLE_STATUS", status_id, 1);
	setenv("PURPLE_STATUS_MSG", status_msg, 1);

	//Launch job on background
	answerscripts_job *job = (answerscripts_job*) malloc(sizeof(answerscripts_job));
	job->pipe = popen(hook_script, "r");
	job->conv = conv;

	#ifndef __WIN32__
		int fflags = fcntl(fileno(job->pipe), F_GETFL, 0);
		fcntl(fileno(job->pipe), F_SETFL, fflags | O_NONBLOCK);
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
	"0.2.2",
	"Framework for hooking scripts to process received messages for libpurple clients",
	"This plugin will execute script ~/.purple/" ANSWERSCRIPT " "
		"or any other executable called  " ANSWERSCRIPT " and found in purple_user_dir() "
		"for each single instant message received.\n"
		"\n- Envinronment values PURPLE_MSG and PURPLE_FROM will be set to carry "
		"informations about message text and sender so script can respond to that message."
		"\n- Any text printed to STDOUT by the script will be sent back as answer to message."
		"\n\nPlease see example scripts, documentation or source code for more informations...",
	"Harvie <harvie@email.cz>",
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
