#define PURPLE_PLUGINS

/* Purple headers */
#include <libpurple/debug.h>
#include <libpurple/version.h>
#include <libpurple/conversation.h>
#include <libpurple/debug.h>
#include <libpurple/log.h>
#include <libpurple/plugin.h>
#include <libpurple/pluginpref.h>
#include <libpurple/prefs.h>
#include <libpurple/signals.h>
#include <libpurple/util.h>
#include <libpurple/notify.h>

#include <stdio.h>
#include <stdlib.h>

#define ANSWERSCRIPT "answerscripts.exe"
#define ANSWERSCRIPTS_TIMEOUT_INTERVAL 250
#define ANSWERSCRIPTS_LINE_LENGTH 4096

char *buff = NULL;
char *hook_script = NULL;
char response[ANSWERSCRIPTS_LINE_LENGTH+1];
int i;

typedef struct {
  FILE *pipe;
  PurpleConversation *conv;
} answerscripts_job;

int answerscripts_process_message(answerscripts_job *job) {
	//TODO: process scripts and send response asynchronously
	FILE *pipe = job->pipe;
	PurpleConversation *conv = job->conv;

	if (pipe && fgets(response, ANSWERSCRIPTS_LINE_LENGTH, pipe)) {
		for(i=0;response[i];i++) if(response[i]=='\n') response[i]=0;
		purple_conv_im_send(purple_conversation_get_im_data(conv), response);
		return 1;
	}
	pclose(pipe);
	free(job);
	return 0;
}

static void
received_im_msg_cb(PurpleAccount *account, char *who, char *buffer, PurpleConversation *conv, PurpleMessageFlags flags, void *data) {

	/* A workaround to avoid skipping of the first message as a result on NULL-conv: */
	if (conv == NULL) conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, account, who);

	buff = purple_markup_strip_html(buffer);
	//printf("\nHarvie received: %s: %s\n", who, buff); //debug
	//purple_conv_im_send(purple_conversation_get_im_data(conv), ":-*"); //debug

	setenv("PURPLE_FROM", who, 1);
	setenv("PURPLE_MSG", buff, 1);


	answerscripts_job *job = (answerscripts_job*) malloc(sizeof(answerscripts_job));
	job->pipe = popen(hook_script, "r");
	job->conv = conv;

	purple_timeout_add(ANSWERSCRIPTS_TIMEOUT_INTERVAL, answerscripts_process_message, (gpointer) job);

}


static gboolean plugin_load(PurplePlugin * plugin) {
	asprintf(&hook_script,"%s/%s",purple_user_dir(),ANSWERSCRIPT);

	void *conv_handle = purple_conversations_get_handle();

	purple_signal_connect(conv_handle, "received-im-msg",
			      plugin, PURPLE_CALLBACK(received_im_msg_cb),
			      NULL);
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
	"0.1",
	"Framework for hooking scripts to received messages for various libpurple clients",
	"This plugin will call ~/.purple/" ANSWERSCRIPT " (or wherever purple_user_dir() points) "
		"script (or any executable) for each single message called."
		"Envinronment values PURPLE_MSG and PURPLE_FROM will be set to carry "
		"informations about message text and sender so script can respond to that message. "
		"Any text printed to STDOUT by the script will be sent back as answer to message. "
		"Please see example scripts for more informations...",
	"Harvie <harvie@email.cz>",
	"http://github.com/harvie",

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
